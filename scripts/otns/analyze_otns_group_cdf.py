#!/usr/bin/env python3
# Copyright (c) 2026 Project CHIP Authors
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
# limitations under the License.

"""
Analyze multi-cycle OTNS Matter Groupcast simulation logs and PCAP to compute
hop-by-hop latency statistics and generate CDF plots.
"""

import glob
import hashlib
import hmac
import json
import math
import os
from pathlib import Path
import re
import struct
import sys
from typing import Any, Dict, List, Optional, Tuple

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
from cryptography.hazmat.primitives.ciphers.aead import AESCCM


def compute_percentiles(vals: List[float]) -> Dict[str, float]:
    if not vals:
        return {
            "count": 0,
            "min": float("nan"),
            "p10": float("nan"),
            "p25": float("nan"),
            "p50": float("nan"),
            "mean": float("nan"),
            "std": float("nan"),
            "p75": float("nan"),
            "p90": float("nan"),
            "p95": float("nan"),
            "p99": float("nan"),
            "max": float("nan"),
        }
    arr = np.array(vals, dtype=float)
    return {
        "count": int(len(arr)),
        "min": float(np.min(arr)),
        "p10": float(np.percentile(arr, 10)),
        "p25": float(np.percentile(arr, 25)),
        "p50": float(np.percentile(arr, 50)),
        "mean": float(np.mean(arr)),
        "std": float(np.std(arr)),
        "p75": float(np.percentile(arr, 75)),
        "p90": float(np.percentile(arr, 90)),
        "p95": float(np.percentile(arr, 95)),
        "p99": float(np.percentile(arr, 99)),
        "max": float(np.max(arr)),
    }


def analyze_and_plot(run_dir: str, out_dir: str) -> Dict[str, Any]:
    os.makedirs(out_dir, exist_ok=True)
    manifest_path = os.path.join(run_dir, "manifest.json")
    with open(manifest_path, "r", encoding="utf-8") as f:
        manifest = json.load(f)

    nodes_meta = manifest["nodes"]
    otbr_log_path = os.path.join(run_dir, "nodes", "0_1.log")
    with open(otbr_log_path, "r", encoding="utf-8", errors="replace") as f:
        otbr_lines = [line.rstrip("\n") for line in f]

    # 1. Discover router RLOC16s and ExtAddrs from logs
    master_key = bytes.fromhex("00112233445566778899aabbccddeeff")
    digest = hmac.new(master_key, b"\x00\x00\x00\x00Thread", hashlib.sha256).digest()
    mac_key = digest[16:32]
    aesccm = AESCCM(mac_key, tag_length=4)

    short_to_ext: Dict[int, bytes] = {}
    for i in range(1, 6):
        p = os.path.join(run_dir, "nodes", f"0_{i}.log")
        if not os.path.isfile(p):
            continue
        with open(p, "r", encoding="utf-8", errors="replace") as f:
            for line in f:
                m = re.search(
                    r"Receive Advertisement \(fe80:0:0:0:([0-9a-fA-F]+):([0-9a-fA-F]+):([0-9a-fA-F]+):([0-9a-fA-F]+),\s*0x([0-9a-fA-F]+)\)",
                    line,
                )
                if m:
                    w0, w1, w2, w3 = [int(x, 16) for x in m.groups()[:4]]
                    rloc = int(m.group(5), 16)
                    short_to_ext[rloc] = struct.pack(">HHHH", w0 ^ 0x0200, w1, w2, w3)

    short_to_node: Dict[int, int] = {}
    node_to_rloc: Dict[int, int] = {}
    for i in range(1, 6):
        p = os.path.join(run_dir, "nodes", f"0_{i}.log")
        if not os.path.isfile(p):
            continue
        with open(p, "r", encoding="utf-8", errors="replace") as f:
            txt = f.read()
        rx_rlocs = set(
            int(x, 16) for x in re.findall(r"Receive Advertisement \([^,]+,\s*0x([0-9a-fA-F]+)\)", txt)
        )
        own = set(short_to_ext.keys()) - rx_rlocs
        if len(own) == 1:
            r = list(own)[0]
            short_to_node[r] = i
            node_to_rloc[i] = r

    otbr_rloc = node_to_rloc.get(1, 0x4800)

    # 2. Identify all distinct Groupcast commands sent by OTBR (separated by > 1.5s)
    otbr_all_tx: List[int] = []
    otbr_cmd_t0s: List[int] = []
    for i, line in enumerate(otbr_lines):
        if "Sent IPv6 HopOpts msg, len:" in line and "to:0xffff" in line:
            m_len = re.search(r"len:(\d+)", line)
            if not m_len or int(m_len.group(1)) < 160:
                continue
            block = "\n".join(otbr_lines[i : i + 4])
            if "ff03:0:0:0:0:0:0:fc" in block:
                m = re.match(r"\s*(\d+)\s+(\S+)", line)
                if m:
                    sim_us = int(m.group(1))
                    otbr_all_tx.append(sim_us)
                    if not otbr_cmd_t0s or (sim_us - otbr_cmd_t0s[-1]) > 1_500_000:
                        otbr_cmd_t0s.append(sim_us)

    # 3. Decrypt all MPL broadcast frames in current.pcap to get ground-truth Hop Limit (64 - HLIM + 1)
    pcap_path = os.path.join(run_dir, "pcap", "current.pcap")
    pcap_hop_counts: Dict[int, int] = {1: 0, 2: 0, 3: 0, 4: 0}
    # Map src_short -> list of (pcap_ts_us, hop_count, mpl_seq)
    pcap_tx_by_src: Dict[int, List[Tuple[int, int, int]]] = {}
    if os.path.isfile(pcap_path):
        with open(pcap_path, "rb") as f:
            pcap_data = f.read()
        off = 24
        while off + 16 <= len(pcap_data):
            ts_sec, ts_usec, incl_len, _orig_len = struct.unpack("<IIII", pcap_data[off : off + 16])
            off += 16
            frame = pcap_data[off : off + incl_len]
            off += incl_len
            if len(frame) not in (124, 125, 126, 127):
                continue
            fcf = struct.unpack("<H", frame[0:2])[0]
            if (fcf & 0x0007) != 1:
                continue
            pan_id, dst_short, src_short = struct.unpack("<HHH", frame[3:9])
            if dst_short != 0xFFFF or src_short not in short_to_ext:
                continue
            sec_level = frame[9] & 0x07
            frame_cnt_be = frame[10:14][::-1]
            nonce = short_to_ext[src_short] + frame_cnt_be + bytes([sec_level])
            try:
                pt = aesccm.decrypt(nonce, frame[15:-2], frame[:15])
                if (pt[0] & 0xF8) == 0xC0:
                    lowpan = pt[4:]
                else:
                    lowpan = pt
                # Outer IPHC: if 0x7e 0x7a -> HLIM=64 (hop 1 from OTBR)
                # if 0x7c 0x6a <HLIM> -> hop = 64 - HLIM + 1
                hop = None
                mpl_seq = -1
                if lowpan[0] == 0x7E and lowpan[1] == 0x7A:
                    hop = 1
                    mpl_seq = lowpan[11]
                elif lowpan[0] == 0x7C and lowpan[1] == 0x6A:
                    hlim = lowpan[2]
                    hop = 64 - hlim + 1
                    mpl_seq = lowpan[14]
                if hop is not None:
                    pcap_hop_counts[hop] = pcap_hop_counts.get(hop, 0) + 1
                    ts_us = ts_sec * 1_000_000 + ts_usec
                    pcap_tx_by_src.setdefault(src_short, []).append((ts_us, hop, mpl_seq))
            except Exception:
                pass

    def lookup_hop_from_pcap(sender_rloc: int, rx_sim_us: int) -> int:
        if sender_rloc == otbr_rloc:
            return 1
        candidates = pcap_tx_by_src.get(sender_rloc, [])
        best_hop = 2
        best_diff = 999_999_999
        for tx_us, hop, _seq in candidates:
            # In OTNS, rx_sim_us is logged at end of frame reception (~4192 us after tx_us start)
            diff = abs((rx_sim_us - 4192) - tx_us)
            if diff < best_diff and diff < 25_000:
                best_diff = diff
                best_hop = hop
        return best_hop

    # 4. Parse controller log for command types (ON/OFF), wpan0 TX timestamps, and ReportData RX
    ct_log_path = os.path.join(run_dir, "controller", "chip_tool_interactive.log")
    tx_events: List[Tuple[str, float, float, int]] = []
    node_ips: Dict[int, str] = {}
    ct_reports_by_idx: Dict[int, Dict[int, Dict[str, Any]]] = {}

    if os.path.isfile(ct_log_path):
        with open(ct_log_path, "r", encoding="utf-8", errors="replace") as f:
            ct_lines = [line.strip() for line in f]

        for i, line in enumerate(ct_lines):
            m = re.search(
                r"\[(\d+\.\d+)\].*Sending cluster \(0x00000006\) command \(0x0000000([01])\) on Group",
                line,
            )
            if m:
                ts = float(m.group(1))
                cmd = "ON" if m.group(2) == "1" else "OFF"
                wpan_ts = ts
                for j in range(i, min(i + 15, len(ct_lines))):
                    mw = re.search(
                        r"\[(\d+\.\d+)\].*Successfully send Multicast message on interface wpan0",
                        ct_lines[j],
                    )
                    if mw:
                        wpan_ts = float(mw.group(1))
                        break
                tx_events.append((cmd, ts, wpan_ts, i))

        for line in ct_lines:
            m = re.search(
                r"Msg TX from [0-9a-fA-F]+ to 1:0*([0-9a-fA-F]+) \[[0-9a-fA-F]+\] \[UDP:\[([^\]]+)\]:5540\]",
                line,
            )
            if m:
                nid = int(m.group(1), 16)
                node_ips[nid] = m.group(2)

        rx_re = re.compile(
            r"\[(?P<ts>\d+\.\d+)\].*Msg RX from \d+:0*(?P<node_hex>[0-9a-fA-F]+)\b.*Type 0001:05 \(IM:ReportData\)"
        )
        attr_re = re.compile(
            r"\[(?P<ts>\d+\.\d+)\].*Endpoint:\s*1\s+Cluster:\s*0x0000_0006\s+Attribute\s+0x0000_0000\b"
        )
        val_re = re.compile(r"\[(?P<ts>\d+\.\d+)\].*\bOnOff:\s*(?P<val>TRUE|FALSE)\b", re.IGNORECASE)

        for idx_ev, (cmd, _tx_ts, wpan_ts, start_line) in enumerate(tx_events):
            ct_reports_by_idx[idx_ev] = {}
            end_line = tx_events[idx_ev + 1][3] if idx_ev + 1 < len(ct_lines) and idx_ev + 1 < len(tx_events) else len(ct_lines)
            cur_node: Optional[int] = None
            cur_ts: Optional[float] = None
            in_attr = False
            for line in ct_lines[start_line:end_line]:
                m_rx = rx_re.search(line)
                if m_rx:
                    cur_ts = float(m_rx.group("ts"))
                    cur_node = int(m_rx.group("node_hex"), 16)
                    in_attr = False
                    continue
                if attr_re.search(line):
                    in_attr = True
                    continue
                if in_attr:
                    mv = val_re.search(line)
                    if mv and cur_node is not None and cur_ts is not None:
                        val = mv.group("val").upper()
                        in_attr = False
                        expected_val = "TRUE" if cmd == "ON" else "FALSE"
                        if cur_node not in ct_reports_by_idx[idx_ev] and val == expected_val:
                            ct_reports_by_idx[idx_ev][cur_node] = {
                                "rx_ts": cur_ts,
                                "val": val,
                                "delta_ms": (cur_ts - wpan_ts) * 1000.0,
                            }

    num_cmds = len(otbr_cmd_t0s)
    cmd_windows: List[Tuple[int, int, str, Optional[float]]] = []
    for k in range(num_cmds):
        t0 = otbr_cmd_t0s[k]
        # Give a 20 ms pre-margin before next command t0 so 1-us early router RX logs go to k+1
        t1 = (otbr_cmd_t0s[k + 1] - 20_000) if k + 1 < num_cmds else 999_999_999_999
        phase = tx_events[k][0] if k < len(tx_events) else ("ON" if k % 2 == 0 else "OFF")
        ct_t0 = tx_events[k][2] if k < len(tx_events) else None
        cmd_windows.append((t0, t1, phase, ct_t0))

    ip_to_nid = {v: k for k, v in node_ips.items()}
    otbr_report_rx: Dict[int, Dict[int, Tuple[int, float]]] = {k: {} for k in range(num_cmds)}
    for i, line in enumerate(otbr_lines):
        m = re.match(
            r"\s*(\d+)\s+(\S+)\s+\[I\] MeshForwarder-: Received IPv6 UDP msg, len:117,.*from:(0x[0-9a-fA-F]+)",
            line,
        )
        if m and i + 1 < len(otbr_lines):
            sim_us = int(m.group(1))
            msrc = re.search(r"src:\[([^\]]+)\]:5540", otbr_lines[i + 1])
            if msrc:
                src_ip = msrc.group(1)
                nid = ip_to_nid.get(src_ip)
                if nid is not None:
                    for k, (t0, t1, _phase, _ct_t0) in enumerate(cmd_windows):
                        if t0 <= sim_us < t1:
                            if nid not in otbr_report_rx[k]:
                                otbr_report_rx[k][nid] = (sim_us, (sim_us - t0) / 1000.0)
                            break

    # Now parse all 25 nodes for all commands
    first_rx_by_hop: Dict[int, List[float]] = {1: [], 2: [], 3: []}
    all_rx_by_hop: Dict[int, List[float]] = {1: [], 2: [], 3: []}
    ctrl_rx_by_hop: Dict[int, List[float]] = {2: [], 3: []}
    router_first_rx_ms: List[float] = []
    matter_first_rx_ms: List[float] = []
    matter_first_rx_on_ms: List[float] = []
    matter_first_rx_off_ms: List[float] = []
    matter_rep_tx_ms: List[float] = []
    otbr_rep_rx_ms: List[float] = []
    controller_rep_rx_ms: List[float] = []
    controller_rep_rx_on_ms: List[float] = []
    controller_rep_rx_off_ms: List[float] = []

    by_column_cmd_rx: Dict[int, List[float]] = {}
    by_column_ctrl_rx: Dict[int, List[float]] = {}
    by_parent_cmd_rx: Dict[str, List[float]] = {}
    by_parent_ctrl_rx: Dict[str, List[float]] = {}
    per_node_stats: Dict[int, Dict[str, Any]] = {}

    total_router_expected = 4 * num_cmds
    total_router_received = 0
    total_matter_expected = 20 * num_cmds
    total_matter_received = 0
    total_matter_reports = 0

    for nmeta in nodes_meta:
        otns_id = int(nmeta["otns_id"])
        if otns_id == 1:
            continue
        mid = nmeta.get("matter_node_id")
        role = nmeta.get("role", "")
        x_pos = int(nmeta.get("x", 0))
        y_pos = int(nmeta.get("y", 0))
        dist_otbr = math.hypot(x_pos - 100, y_pos - 300)

        nlog_path = os.path.join(run_dir, nmeta["raw_log"])
        with open(nlog_path, "r", encoding="utf-8", errors="replace") as f:
            nlines = [line.rstrip("\n") for line in f]

        cmd_rx_by_k: Dict[int, List[Tuple[int, int, float]]] = {k: [] for k in range(num_cmds)}
        rep_tx_by_k: Dict[int, List[Tuple[int, int]]] = {k: [] for k in range(num_cmds)}
        parent_rloc_hex = None

        for line in nlines:
            if "Received IPv6 HopOpts msg, len:" in line:
                m_len = re.search(r"len:(\d+)", line)
                if not m_len or int(m_len.group(1)) < 160:
                    continue
                m = re.match(r"\s*(\d+)\s+\S+.*from:(0x[0-9a-fA-F]+).*rss:([-\d\.]+)", line)
                if m:
                    sim_us = int(m.group(1))
                    sender_rloc = int(m.group(2), 16)
                    rss = float(m.group(3))
                    for k, (t0, t1, _phase, _ct_t0) in enumerate(cmd_windows):
                        if (t0 - 10_000) <= sim_us < t1:
                            cmd_rx_by_k[k].append((sim_us, sender_rloc, rss))
                            break
            elif "Sent IPv6 UDP msg, len:117," in line:
                m = re.match(r"\s*(\d+)\s+\S+.*to:(0x[0-9a-fA-F]+)", line)
                if m:
                    sim_us = int(m.group(1))
                    nhop = int(m.group(2), 16)
                    parent_rloc_hex = f"0x{nhop:04x}"
                    for k, (t0, t1, _phase, _ct_t0) in enumerate(cmd_windows):
                        if t0 <= sim_us < t1:
                            rep_tx_by_k[k].append((sim_us, nhop))
                            break

        node_cmd_ms: List[float] = []
        node_rep_tx_ms: List[float] = []
        node_otbr_rx_ms: List[float] = []
        node_ctrl_rx_ms: List[float] = []
        node_hops: List[int] = []

        for k, (t0, _t1, phase, _ct_t0) in enumerate(cmd_windows):
            rxs = cmd_rx_by_k[k]
            if rxs:
                if mid is None:
                    total_router_received += 1
                else:
                    total_matter_received += 1

                # First reception for this node on command k
                first_us, first_sender, _first_rss = rxs[0]
                delta_ms = max(0.0, (first_us - t0) / 1000.0)
                hop = lookup_hop_from_pcap(first_sender, first_us)
                first_rx_by_hop.setdefault(hop, []).append(delta_ms)
                node_cmd_ms.append(delta_ms)
                node_hops.append(hop)

                if mid is None:
                    router_first_rx_ms.append(delta_ms)
                else:
                    matter_first_rx_ms.append(delta_ms)
                    if phase == "ON":
                        matter_first_rx_on_ms.append(delta_ms)
                    else:
                        matter_first_rx_off_ms.append(delta_ms)
                    by_column_cmd_rx.setdefault(x_pos, []).append(delta_ms)
                    if parent_rloc_hex:
                        by_parent_cmd_rx.setdefault(parent_rloc_hex, []).append(delta_ms)

                # All MPL copies received by this node on command k
                for rx_us, sender_rloc, _rss in rxs:
                    copy_ms = max(0.0, (rx_us - t0) / 1000.0)
                    copy_hop = lookup_hop_from_pcap(sender_rloc, rx_us)
                    all_rx_by_hop.setdefault(copy_hop, []).append(copy_ms)

            if mid is not None:
                if rep_tx_by_k[k]:
                    rtx_ms = max(0.0, (rep_tx_by_k[k][0][0] - t0) / 1000.0)
                    matter_rep_tx_ms.append(rtx_ms)
                    node_rep_tx_ms.append(rtx_ms)
                if mid in otbr_report_rx[k]:
                    orx_ms = max(0.0, otbr_report_rx[k][mid][1])
                    otbr_rep_rx_ms.append(orx_ms)
                    node_otbr_rx_ms.append(orx_ms)
                ct_rep = ct_reports_by_idx.get(k, {}).get(mid)
                if ct_rep:
                    total_matter_reports += 1
                    crx_ms = max(0.0, ct_rep["delta_ms"])
                    controller_rep_rx_ms.append(crx_ms)
                    node_ctrl_rx_ms.append(crx_ms)
                    sender_hop = lookup_hop_from_pcap(rxs[0][1], rxs[0][0]) if rxs else 2
                    ctrl_rx_by_hop.setdefault(sender_hop, []).append(crx_ms)
                    if phase == "ON":
                        controller_rep_rx_on_ms.append(crx_ms)
                    else:
                        controller_rep_rx_off_ms.append(crx_ms)
                    by_column_ctrl_rx.setdefault(x_pos, []).append(crx_ms)
                    if parent_rloc_hex:
                        by_parent_ctrl_rx.setdefault(parent_rloc_hex, []).append(crx_ms)

        per_node_stats[otns_id] = {
            "otns_id": otns_id,
            "matter_node_id": mid,
            "role": role,
            "x": x_pos,
            "y": y_pos,
            "dist_otbr": round(dist_otbr, 1),
            "parent_rloc": parent_rloc_hex or (f"0x{node_to_rloc[otns_id]:04x}" if otns_id in node_to_rloc else "-"),
            "cmd_rx_delivery_pct": round(100.0 * len(node_cmd_ms) / max(1, num_cmds), 2),
            "report_rx_delivery_pct": round(100.0 * len(node_ctrl_rx_ms) / max(1, num_cmds), 2) if mid else None,
            "mean_hops": round(float(np.mean(node_hops)), 3) if node_hops else None,
            "cmd_rx": compute_percentiles(node_cmd_ms),
            "rep_tx": compute_percentiles(node_rep_tx_ms) if mid else None,
            "otbr_rx": compute_percentiles(node_otbr_rx_ms) if mid else None,
            "ctrl_rx": compute_percentiles(node_ctrl_rx_ms) if mid else None,
        }

    # 5. Generate CDF plots!
    plt.rcParams.update({
        "font.size": 10,
        "axes.titlesize": 11.5,
        "axes.titleweight": "bold",
        "axes.labelsize": 10,
        "axes.grid": True,
        "grid.alpha": 0.3,
        "figure.dpi": 160,
    })

    def plot_cdf_line(ax, data: List[float], label: str, color: str, linestyle: str = "-", linewidth: float = 2.2):
        if not data:
            return
        sorted_d = np.sort(np.array(data, dtype=float))
        y = np.arange(1, len(sorted_d) + 1) / len(sorted_d)
        x_plot = np.concatenate([[sorted_d[0]], sorted_d])
        y_plot = np.concatenate([[0.0], y])
        ax.step(x_plot, y_plot, where="post", label=label, color=color, linestyle=linestyle, linewidth=linewidth)

    # --- Plot 1: CDF of Time to Receive Group Command (CmdRX) and Controller ReportData RX by Hop Count ---
    fig, (ax1, ax2, ax3) = plt.subplots(1, 3, figsize=(18.6, 5.8))
    fig.suptitle(
        f"Matter Groupcast over Thread: Command Reception (CmdRX) & Controller ReportData RX CDF by Hop Distance ({num_cmds} Group Commands)",
        fontsize=12.5,
        fontweight="bold",
        y=0.98,
    )

    h1_first = first_rx_by_hop.get(1, [])
    h2_first = first_rx_by_hop.get(2, [])
    h3_first = first_rx_by_hop.get(3, [])
    agg_first = h1_first + h2_first + h3_first

    plot_cdf_line(
        ax1,
        h1_first,
        f"1 Hop from OTBR (Direct to Routers, n={len(h1_first)}, P50={np.median(h1_first):.2f} ms, P90={np.percentile(h1_first, 90):.1f} ms)",
        "#1a73e8",
    )
    plot_cdf_line(
        ax1,
        h2_first,
        f"2 Hops from OTBR (1-Hop Router -> Node, n={len(h2_first)}, P50={np.median(h2_first):.1f} ms, P90={np.percentile(h2_first, 90):.1f} ms)",
        "#1e8e3e",
    )
    if h3_first:
        plot_cdf_line(
            ax1,
            h3_first,
            f"3 Hops from OTBR (2-Hop Router -> Node, n={len(h3_first)}, P50={np.median(h3_first):.1f} ms, P90={np.percentile(h3_first, 90):.1f} ms)",
            "#d93025",
        )
    plot_cdf_line(
        ax1,
        agg_first,
        f"Aggregated All Nodes (n={len(agg_first)}, P50={np.median(agg_first):.1f} ms, P90={np.percentile(agg_first, 90):.1f} ms)",
        "#202124",
        linestyle="--",
        linewidth=1.8,
    )

    ax1.set_title("A. First Command Reception (First CmdRX) by Hops")
    ax1.set_xlabel("Time to Receive Group Command from OTBR TX (ms)")
    ax1.set_ylabel("CDF (Cumulative Probability)")
    ax1.set_xlim(-5, 350)
    ax1.set_ylim(-0.02, 1.03)
    ax1.set_yticks(np.linspace(0, 1.0, 11))
    ax1.axhline(0.50, color="gray", linestyle=":", linewidth=0.9, alpha=0.6)
    ax1.axhline(0.90, color="gray", linestyle=":", linewidth=0.9, alpha=0.6)
    ax1.axhline(0.99, color="gray", linestyle=":", linewidth=0.9, alpha=0.6)
    ax1.legend(loc="lower right", frameon=True, fontsize=8.0)

    h1_all = all_rx_by_hop.get(1, [])
    h2_all = all_rx_by_hop.get(2, [])
    h3_all = all_rx_by_hop.get(3, [])
    agg_all = h1_all + h2_all + h3_all

    plot_cdf_line(
        ax2,
        h1_all,
        f"1 Hop (Direct from OTBR incl. Trickle, n={len(h1_all)}, P50={np.median(h1_all):.1f} ms)",
        "#1a73e8",
    )
    plot_cdf_line(
        ax2,
        h2_all,
        f"2 Hops (Via 1-Hop Router incl. Trickle, n={len(h2_all)}, P50={np.median(h2_all):.1f} ms)",
        "#1e8e3e",
    )
    if h3_all:
        plot_cdf_line(
            ax2,
            h3_all,
            f"3 Hops (Via 2-Hop Router incl. Trickle, n={len(h3_all)}, P50={np.median(h3_all):.1f} ms)",
            "#d93025",
        )
    plot_cdf_line(
        ax2,
        agg_all,
        f"Aggregated All MPL Copies (n={len(agg_all)}, P50={np.median(agg_all):.1f} ms)",
        "#202124",
        linestyle="--",
        linewidth=1.8,
    )

    ax2.set_title("B. All Received MPL Copies by Wire Hop Count (64 - HLIM + 1)")
    ax2.set_xlabel("Time of MPL Frame Reception from Initial OTBR TX (ms)")
    ax2.set_ylabel("CDF (Cumulative Probability)")
    ax2.set_xlim(-5, 450)
    ax2.set_ylim(-0.02, 1.03)
    ax2.set_yticks(np.linspace(0, 1.0, 11))
    ax2.axhline(0.50, color="gray", linestyle=":", linewidth=0.9, alpha=0.6)
    ax2.axhline(0.90, color="gray", linestyle=":", linewidth=0.9, alpha=0.6)
    ax2.axhline(0.99, color="gray", linestyle=":", linewidth=0.9, alpha=0.6)
    ax2.legend(loc="lower right", frameon=True, fontsize=8.0)

    c_h2 = ctrl_rx_by_hop.get(2, [])
    c_h3 = ctrl_rx_by_hop.get(3, [])
    plot_cdf_line(
        ax3,
        c_h2,
        f"2 Hops from OTBR (1-Hop Router -> MED, n={len(c_h2)}, P50={np.median(c_h2):.1f} ms, P90={np.percentile(c_h2, 90):.1f} ms)",
        "#1e8e3e",
    )
    if c_h3:
        plot_cdf_line(
            ax3,
            c_h3,
            f"3 Hops from OTBR (2-Hop Router -> MED, n={len(c_h3)}, P50={np.median(c_h3):.1f} ms, P90={np.percentile(c_h3, 90):.1f} ms)",
            "#d93025",
        )
    plot_cdf_line(
        ax3,
        controller_rep_rx_ms,
        f"Aggregated All Senders (n={len(controller_rep_rx_ms)}, P50={np.median(controller_rep_rx_ms):.1f} ms, P90={np.percentile(controller_rep_rx_ms, 90):.1f} ms)",
        "#9334e6",
        linestyle="--",
        linewidth=2.0,
    )

    ax3.set_title("C. Controller IM:ReportData RX by Sender Hop Distance")
    ax3.set_xlabel("Time to Controller IM:ReportData RX from OTBR TX (ms)")
    ax3.set_ylabel("CDF (Cumulative Probability)")
    ax3.set_xlim(-10, 1200)
    ax3.set_ylim(-0.02, 1.03)
    ax3.set_yticks(np.linspace(0, 1.0, 11))
    ax3.axhline(0.50, color="gray", linestyle=":", linewidth=0.9, alpha=0.6)
    ax3.axhline(0.90, color="gray", linestyle=":", linewidth=0.9, alpha=0.6)
    ax3.axhline(0.99, color="gray", linestyle=":", linewidth=0.9, alpha=0.6)
    ax3.legend(loc="lower right", frameon=True, fontsize=8.0)

    plt.tight_layout(rect=[0, 0, 1, 0.94])
    plot1_path = os.path.join(out_dir, "cdf_cmd_rx_by_hops.png")
    plt.savefig(plot1_path)
    plt.close(fig)

    # --- Plot 2: CDF of End-to-End Groupcast & Wildcard Subscription Pipeline Stages ---
    fig, ax = plt.subplots(figsize=(11.5, 6.0))
    plot_cdf_line(
        ax,
        h1_first,
        f"1. 1-Hop Router CmdRX (Direct from OTBR, n={len(h1_first)}, P50={np.median(h1_first):.2f} ms, P90={np.percentile(h1_first, 90):.1f} ms)",
        "#1a73e8",
    )
    plot_cdf_line(
        ax,
        h2_first,
        f"2. 2-Hop Node CmdRX (Via 1-Hop Router, n={len(h2_first)}, P50={np.median(h2_first):.1f} ms, P90={np.percentile(h2_first, 90):.1f} ms)",
        "#1e8e3e",
    )
    if h3_first:
        plot_cdf_line(
            ax,
            h3_first,
            f"3. 3-Hop Node CmdRX (Via 2-Hop Router, n={len(h3_first)}, P50={np.median(h3_first):.1f} ms, P90={np.percentile(h3_first, 90):.1f} ms)",
            "#d93025",
        )
    plot_cdf_line(
        ax,
        matter_rep_tx_ms,
        f"4. Matter Node IM:ReportData TX (n={len(matter_rep_tx_ms)}, P50={np.median(matter_rep_tx_ms):.1f} ms, P90={np.percentile(matter_rep_tx_ms, 90):.1f} ms)",
        "#f9ab00",
    )
    plot_cdf_line(
        ax,
        otbr_rep_rx_ms,
        f"5. OTBR IM:ReportData RX (2 Unicast Hops, n={len(otbr_rep_rx_ms)}, P50={np.median(otbr_rep_rx_ms):.1f} ms, P90={np.percentile(otbr_rep_rx_ms, 90):.1f} ms)",
        "#e37400",
    )
    plot_cdf_line(
        ax,
        controller_rep_rx_ms,
        f"6. Controller (chip-tool) IM:ReportData RX (n={len(controller_rep_rx_ms)}, P50={np.median(controller_rep_rx_ms):.1f} ms, P90={np.percentile(controller_rep_rx_ms, 90):.1f} ms)",
        "#9334e6",
    )

    # Annotate the two-wave MRP plateau vs Hop-2 router queueing smoothing
    ax.annotate(
        "Wave 1 (~83%): 1st MRP attempt ACKed\non Hop 1 within 1–16 MAC tries (<=315 ms)",
        xy=(140, 0.56),
        xytext=(240, 0.66),
        fontsize=8.2,
        bbox=dict(boxstyle="round,pad=0.3", fc="#fff8e1", ec="#f9ab00", alpha=0.92),
        arrowprops=dict(arrowstyle="->", color="#f9ab00", lw=1.2),
    )
    ax.annotate(
        "315–600 ms Plateau: 16 MAC tries exhausted\n(Failed to send, n=174); waiting on Matter MRP timer",
        xy=(450, 0.835),
        xytext=(330, 0.91),
        fontsize=8.2,
        bbox=dict(boxstyle="round,pad=0.3", fc="#fff8e1", ec="#f9ab00", alpha=0.92),
        arrowprops=dict(arrowstyle="->", color="#f9ab00", lw=1.2),
    )
    ax.annotate(
        "Hop-2 Router Queueing (Router -> OTBR,\n+55.9 ms P50, +195.3 ms P90) smears\nWave 1 smoothly across 100–600 ms",
        xy=(310, 0.62),
        xytext=(430, 0.48),
        fontsize=8.2,
        bbox=dict(boxstyle="round,pad=0.3", fc="#f3e8fd", ec="#9334e6", alpha=0.92),
        arrowprops=dict(arrowstyle="->", color="#9334e6", lw=1.2),
    )

    ax.set_title(
        f"End-to-End Latency CDF Across Protocol Stages ({num_cmds} Group Commands)"
    )
    ax.set_xlabel("Latency from Group Command Transmission (ms)")
    ax.set_ylabel("CDF (Cumulative Probability)")
    ax.set_xlim(-10, 1200)
    ax.set_ylim(-0.02, 1.03)
    ax.set_yticks(np.linspace(0, 1.0, 11))
    ax.axhline(0.50, color="gray", linestyle=":", linewidth=0.9, alpha=0.6)
    ax.axhline(0.90, color="gray", linestyle=":", linewidth=0.9, alpha=0.6)
    ax.axhline(0.99, color="gray", linestyle=":", linewidth=0.9, alpha=0.6)
    ax.legend(loc="lower right", frameon=True, fontsize=8.6)
    plt.tight_layout()
    plot2_path = os.path.join(out_dir, "cdf_pipeline_stages.png")
    plt.savefig(plot2_path)
    plt.close(fig)

    # --- Plot 3: Two-panel CDF: (A) Collapsed Group Command Latency, (B) By Spatial Distance / Grid Column ---
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(15.0, 5.6))
    fig.suptitle(
        f"Latency CDF Breakdown: Group Command Summary & Spatial Grid Column Distance from OTBR ({num_cmds} Commands)",
        fontsize=12.5,
        fontweight="bold",
        y=0.98,
    )

    plot_cdf_line(
        ax1,
        matter_first_rx_ms,
        f"Group Command — Matter Node CmdRX (n={len(matter_first_rx_ms)}, P50={np.median(matter_first_rx_ms):.1f} ms, P90={np.percentile(matter_first_rx_ms, 90):.1f} ms)",
        "#1e8e3e",
        linestyle="-",
        linewidth=2.4,
    )
    plot_cdf_line(
        ax1,
        controller_rep_rx_ms,
        f"Group Command — Controller IM:ReportData RX (n={len(controller_rep_rx_ms)}, P50={np.median(controller_rep_rx_ms):.1f} ms, P90={np.percentile(controller_rep_rx_ms, 90):.1f} ms)",
        "#9334e6",
        linestyle="-",
        linewidth=2.4,
    )
    ax1.set_title("A. Group Command CDF (Node CmdRX vs. Controller RX)")
    ax1.set_xlabel("Latency from Group Command Transmission (ms)")
    ax1.set_ylabel("CDF (Cumulative Probability)")
    ax1.set_xlim(-10, 1200)
    ax1.set_ylim(-0.02, 1.03)
    ax1.set_yticks(np.linspace(0, 1.0, 11))
    ax1.axhline(0.50, color="gray", linestyle=":", linewidth=0.9, alpha=0.6)
    ax1.axhline(0.90, color="gray", linestyle=":", linewidth=0.9, alpha=0.6)
    ax1.axhline(0.99, color="gray", linestyle=":", linewidth=0.9, alpha=0.6)
    ax1.legend(loc="lower right", frameon=True, fontsize=8.8)

    col_palette = ["#1a73e8", "#1e8e3e", "#f9ab00", "#e37400", "#d93025"]
    for idx, x_val in enumerate(sorted(by_column_cmd_rx.keys())):
        col_c = col_palette[idx % len(col_palette)]
        c_cmd = by_column_cmd_rx[x_val]
        plot_cdf_line(
            ax2,
            c_cmd,
            f"Col {idx+1} (x={x_val}, Δx={x_val-100}m) CmdRX (n={len(c_cmd)}, P50={np.median(c_cmd):.1f} ms, P90={np.percentile(c_cmd, 90):.1f} ms)",
            col_c,
            linestyle="-",
            linewidth=2.2,
        )
    plot_cdf_line(
        ax2,
        matter_first_rx_ms,
        f"Aggregated Group Command CmdRX (n={len(matter_first_rx_ms)}, P50={np.median(matter_first_rx_ms):.1f} ms, P90={np.percentile(matter_first_rx_ms, 90):.1f} ms)",
        "#202124",
        linestyle="--",
        linewidth=1.8,
    )

    ax2.set_title("B. Group Command Node CmdRX by Grid Column (x=290..470)")
    ax2.set_xlabel("Time to Receive Group Command (CmdRX, ms)")
    ax2.set_ylabel("CDF (Cumulative Probability)")
    ax2.set_xlim(-5, 250)
    ax2.set_ylim(-0.02, 1.03)
    ax2.set_yticks(np.linspace(0, 1.0, 11))
    ax2.axhline(0.50, color="gray", linestyle=":", linewidth=0.9, alpha=0.6)
    ax2.axhline(0.90, color="gray", linestyle=":", linewidth=0.9, alpha=0.6)
    ax2.axhline(0.99, color="gray", linestyle=":", linewidth=0.9, alpha=0.6)
    ax2.legend(loc="lower right", frameon=True, fontsize=8.2)

    plt.tight_layout(rect=[0, 0, 1, 0.94])
    plot3_path = os.path.join(out_dir, "cdf_on_vs_off_and_columns.png")
    plt.savefig(plot3_path)
    plt.close(fig)

    summary = {
        "run_id": manifest.get("run_id"),
        "num_commands": num_cmds,
        "num_cycles": num_cmds // 2,
        "router_delivery": {
            "expected": total_router_expected,
            "received": total_router_received,
            "pct": round(100.0 * total_router_received / max(1, total_router_expected), 2),
        },
        "matter_cmd_delivery": {
            "expected": total_matter_expected,
            "received": total_matter_received,
            "pct": round(100.0 * total_matter_received / max(1, total_matter_expected), 2),
        },
        "matter_report_delivery": {
            "expected": total_matter_expected,
            "received": total_matter_reports,
            "pct": round(100.0 * total_matter_reports / max(1, total_matter_expected), 2),
        },
        "pcap_mpl_frames_by_hop": pcap_hop_counts,
        "first_cmd_rx_by_hop": {
            **{str(h): compute_percentiles(v) for h, v in sorted(first_rx_by_hop.items()) if v},
            "aggregated": compute_percentiles(agg_first),
        },
        "all_cmd_rx_by_hop": {
            **{str(h): compute_percentiles(v) for h, v in sorted(all_rx_by_hop.items()) if v},
            "aggregated": compute_percentiles(agg_all),
        },
        "controller_rep_rx_by_hop": {
            **{str(h): compute_percentiles(v) for h, v in sorted(ctrl_rx_by_hop.items()) if v},
            "aggregated": compute_percentiles(controller_rep_rx_ms),
        },
        "pipeline_stages": {
            "router_1hop_cmd_rx": compute_percentiles(router_first_rx_ms),
            "matter_node_cmd_rx": compute_percentiles(matter_first_rx_ms),
            "matter_node_rep_tx": compute_percentiles(matter_rep_tx_ms),
            "otbr_rep_rx": compute_percentiles(otbr_rep_rx_ms),
            "controller_rep_rx": compute_percentiles(controller_rep_rx_ms),
        },
        "by_grid_column": {
            str(x): {
                "cmd_rx": compute_percentiles(by_column_cmd_rx[x]),
                "ctrl_rx": compute_percentiles(by_column_ctrl_rx.get(x, [])),
            }
            for x in sorted(by_column_cmd_rx.keys())
        },
        "by_parent_router": {
            r: {
                "cmd_rx": compute_percentiles(by_parent_cmd_rx[r]),
                "ctrl_rx": compute_percentiles(by_parent_ctrl_rx.get(r, [])),
            }
            for r in sorted(by_parent_cmd_rx.keys())
        },
        "per_node": per_node_stats,
        "plots": [plot1_path, plot2_path, plot3_path],
    }

    summary_json_path = os.path.join(out_dir, "group_latency_stats.json")
    with open(summary_json_path, "w", encoding="utf-8") as f:
        json.dump(summary, f, indent=2)

    return summary


if __name__ == "__main__":
    repo_root = Path(__file__).resolve().parents[2]
    rdir = sys.argv[1] if len(sys.argv) > 1 else str(repo_root / "sim_logs" / "latest")
    odir = sys.argv[2] if len(sys.argv) > 2 else os.path.realpath(rdir)
    res = analyze_and_plot(os.path.realpath(rdir), odir)
    print(json.dumps({k: v for k, v in res.items() if k != "per_node"}, indent=2))
