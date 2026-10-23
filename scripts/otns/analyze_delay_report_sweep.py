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
Comparative analysis of Matter Groupcast DelayReportData (delayMinMs, delayJitterWindowMs)
simulations over OpenThread (OTNS):
  1. Controller IM:ReportData RX latency CDFs (absolute, post-deferral, and router queueing delay)
  2. Comprehensive packet drop & MAC congestion breakdown across runs
"""

import glob
import json
import os
from pathlib import Path
import re
import sys
from typing import Any, Dict, List, Optional, Tuple

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np


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


def parse_single_run(run_dir: str, label: str, short_label: str, color: str, linestyle: str = "-") -> Dict[str, Any]:
    manifest_path = os.path.join(run_dir, "manifest.json")
    with open(manifest_path, "r", encoding="utf-8") as f:
        manifest = json.load(f)

    dmin = manifest.get("delay_report_min_ms")
    djit = manifest.get("delay_report_jitter_ms")
    dmin_val = int(dmin) if dmin is not None else 0
    djit_val = int(djit) if djit is not None else 0

    nodes_meta = manifest["nodes"]
    otbr_log_path = os.path.join(run_dir, "nodes", "0_1.log")
    with open(otbr_log_path, "r", encoding="utf-8", errors="replace") as f:
        otbr_lines = [line.rstrip("\n") for line in f]

    otbr_cmd_t0s: List[int] = []
    mpl_pkt_len: Optional[int] = None
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
                    mpl_pkt_len = int(m_len.group(1))
                    if not otbr_cmd_t0s or (sim_us - otbr_cmd_t0s[-1]) > 1_500_000:
                        otbr_cmd_t0s.append(sim_us)

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
            end_line = tx_events[idx_ev + 1][3] if idx_ev + 1 < len(tx_events) else len(ct_lines)
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

    matter_first_rx_ms: List[float] = []
    matter_rep_tx_ms: List[float] = []
    otbr_rep_rx_ms: List[float] = []
    controller_rep_rx_ms: List[float] = []
    post_deferral_ctrl_rx_ms: List[float] = []
    router_queue_delay_ms: List[float] = []

    total_router_expected = 4 * num_cmds
    total_router_received = 0
    total_matter_expected = 20 * num_cmds
    total_matter_received = 0
    total_matter_reports_rx = 0
    total_mrp_extra_tx = 0

    for nmeta in nodes_meta:
        otns_id = int(nmeta["otns_id"])
        if otns_id == 1:
            continue
        mid = nmeta.get("matter_node_id")
        nlog_path = os.path.join(run_dir, nmeta["raw_log"])
        with open(nlog_path, "r", encoding="utf-8", errors="replace") as f:
            nlines = [line.rstrip("\n") for line in f]

        cmd_rx_by_k: Dict[int, List[int]] = {k: [] for k in range(num_cmds)}
        rep_tx_by_k: Dict[int, List[int]] = {k: [] for k in range(num_cmds)}

        for line in nlines:
            if "Received IPv6 HopOpts msg, len:" in line:
                m_len = re.search(r"len:(\d+)", line)
                if not m_len or int(m_len.group(1)) < 160:
                    continue
                m = re.match(r"\s*(\d+)\s+", line)
                if m:
                    sim_us = int(m.group(1))
                    for k, (t0, t1, _phase, _ct_t0) in enumerate(cmd_windows):
                        if (t0 - 10_000) <= sim_us < t1:
                            cmd_rx_by_k[k].append(sim_us)
                            break
            elif "Sent IPv6 UDP msg, len:117," in line:
                m = re.match(r"\s*(\d+)\s+", line)
                if m:
                    sim_us = int(m.group(1))
                    for k, (t0, t1, _phase, _ct_t0) in enumerate(cmd_windows):
                        if t0 <= sim_us < t1:
                            rep_tx_by_k[k].append(sim_us)
                            break

        for k, (t0, _t1, _phase, _ct_t0) in enumerate(cmd_windows):
            rxs = cmd_rx_by_k[k]
            if rxs:
                if mid is None:
                    total_router_received += 1
                else:
                    total_matter_received += 1
                    matter_first_rx_ms.append(max(0.0, (rxs[0] - t0) / 1000.0))

            if mid is not None:
                txs = rep_tx_by_k[k]
                rtx_ms: Optional[float] = None
                if txs:
                    rtx_ms = max(0.0, (txs[0] - t0) / 1000.0)
                    matter_rep_tx_ms.append(rtx_ms)
                    if len(txs) > 1:
                        total_mrp_extra_tx += len(txs) - 1

                if mid in otbr_report_rx[k]:
                    orx_ms = max(0.0, otbr_report_rx[k][mid][1])
                    otbr_rep_rx_ms.append(orx_ms)
                    if rtx_ms is not None and orx_ms >= rtx_ms:
                        router_queue_delay_ms.append(orx_ms - rtx_ms)

                ct_rep = ct_reports_by_idx.get(k, {}).get(mid)
                if ct_rep:
                    total_matter_reports_rx += 1
                    crx_ms = max(0.0, ct_rep["delta_ms"])
                    controller_rep_rx_ms.append(crx_ms)
                    post_deferral_ctrl_rx_ms.append(max(0.0, crx_ms - dmin_val))

    # Count packet drop & MAC error events across all 25 nodes during the Groupcast phase (ts >= first_cmd_t0 - 10ms)
    start_us = otbr_cmd_t0s[0] - 10_000 if otbr_cmd_t0s else 0
    drops = {
        "missed_matter_cmd_rx": total_matter_expected - total_matter_received,
        "missed_router_cmd_rx": total_router_expected - total_router_received,
        "missed_controller_rep_rx": total_matter_expected - total_matter_reports_rx,
        "mrp_retransmissions": total_mrp_extra_tx,
        "nobufs_drops": 0,
        "queue_mgmt_mesh_drops": 0,
        "queue_mgmt_hopopts_drops": 0,
        "failed_tx_udp_drops": 0,
        "failed_tx_mesh_drops": 0,
        "mpl_reassembly_frag_drops": 0,
        "unicast_reassembly_frag_drops": 0,
        "reassembly_frag_drops": 0,
        "mac_fcs_err": 0,
        "mac_tx_noack": 0,
        "mac_tx_cca_fail": 0,
        "mac_rx_duplicated": 0,
    }

    for p in sorted(glob.glob(os.path.join(run_dir, "nodes", "0_*.log"))):
        with open(p, "r", encoding="utf-8", errors="replace") as f:
            for line in f:
                m = re.match(r"^\s*(\d+)\s+\S+\s+\[[IWN]\]\s+(\S+):\s+(.*)", line)
                if not m:
                    continue
                ts = int(m.group(1))
                if ts < start_us:
                    continue
                mod, msg = m.group(2), m.group(3)
                if mod == "MeshForwarder-":
                    if "error:NoBufs" in msg:
                        drops["nobufs_drops"] += 1
                    elif "Dropping (queue mgmt) mesh frame" in msg:
                        drops["queue_mgmt_mesh_drops"] += 1
                    elif "Dropping (queue mgmt) IPv6 HopOpts msg" in msg:
                        drops["queue_mgmt_hopopts_drops"] += 1
                    elif "Failed to send IPv6 UDP msg" in msg:
                        drops["failed_tx_udp_drops"] += 1
                    elif "Failed to send mesh frame" in msg:
                        drops["failed_tx_mesh_drops"] += 1
                    elif "error:ReassemblyTimeout" in msg or "Dropping rx frag frame, error:Drop" in msg:
                        drops["reassembly_frag_drops"] += 1
                        if "IPv6 HopOpts msg" in msg or "dst:0xffff" in msg:
                            drops["mpl_reassembly_frag_drops"] += 1
                        else:
                            drops["unicast_reassembly_frag_drops"] += 1
                elif mod == "Mac-----------":
                    if "Frame rx failed, error:FcsErr" in msg:
                        drops["mac_fcs_err"] += 1
                    elif "Frame tx attempt" in msg and "error:NoAck" in msg:
                        drops["mac_tx_noack"] += 1
                    elif "Frame tx attempt" in msg and "error:ChannelAccessFailure" in msg:
                        drops["mac_tx_cca_fail"] += 1
                    elif "Frame rx failed, error:Duplicated" in msg:
                        drops["mac_rx_duplicated"] += 1

    drops["unicast_congestion_pkt_drops"] = (
        drops["nobufs_drops"]
        + drops["queue_mgmt_mesh_drops"]
        + drops["failed_tx_udp_drops"]
        + drops["failed_tx_mesh_drops"]
        + drops["unicast_reassembly_frag_drops"]
    )
    drops["total_ipv6_mesh_pkt_drops"] = (
        drops["unicast_congestion_pkt_drops"]
        + drops["queue_mgmt_hopopts_drops"]
        + drops["mpl_reassembly_frag_drops"]
    )

    per_cmd_drops = {k: round(v / max(1, num_cmds), 3) for k, v in drops.items()}

    return {
        "run_id": manifest["run_id"],
        "run_dir": run_dir,
        "label": label,
        "short_label": short_label,
        "color": color,
        "linestyle": linestyle,
        "delay_min_ms": dmin_val,
        "delay_jitter_ms": djit_val,
        "mpl_pkt_len": mpl_pkt_len,
        "num_cmds": num_cmds,
        "expected_node_cmds": total_matter_expected,
        "cmd_rx_delivery_pct": round(100.0 * total_matter_received / max(1, total_matter_expected), 2),
        "ctrl_rx_delivery_pct": round(100.0 * total_matter_reports_rx / max(1, total_matter_expected), 2),
        "raw_data": {
            "matter_first_rx_ms": matter_first_rx_ms,
            "matter_rep_tx_ms": matter_rep_tx_ms,
            "otbr_rep_rx_ms": otbr_rep_rx_ms,
            "controller_rep_rx_ms": controller_rep_rx_ms,
            "post_deferral_ctrl_rx_ms": post_deferral_ctrl_rx_ms,
            "router_queue_delay_ms": router_queue_delay_ms,
        },
        "stats": {
            "matter_first_rx_ms": compute_percentiles(matter_first_rx_ms),
            "matter_rep_tx_ms": compute_percentiles(matter_rep_tx_ms),
            "otbr_rep_rx_ms": compute_percentiles(otbr_rep_rx_ms),
            "controller_rep_rx_ms": compute_percentiles(controller_rep_rx_ms),
            "post_deferral_ctrl_rx_ms": compute_percentiles(post_deferral_ctrl_rx_ms),
            "router_queue_delay_ms": compute_percentiles(router_queue_delay_ms),
        },
        "drops_total": drops,
        "drops_per_cmd": per_cmd_drops,
    }


def plot_cdf_line(ax, data: List[float], label: str, color: str, linestyle: str = "-", linewidth: float = 2.2):
    if not data:
        return
    sorted_d = np.sort(np.array(data, dtype=float))
    y = np.arange(1, len(sorted_d) + 1) / len(sorted_d)
    x_plot = np.concatenate([[sorted_d[0]], sorted_d])
    y_plot = np.concatenate([[0.0], y])
    ax.step(x_plot, y_plot, where="post", label=label, color=color, linestyle=linestyle, linewidth=linewidth)


def generate_comparison_plots_and_stats(runs: List[Dict[str, Any]], out_dir: str) -> None:
    os.makedirs(out_dir, exist_ok=True)
    plt.rcParams.update({
        "font.size": 10,
        "axes.titlesize": 11.5,
        "axes.titleweight": "bold",
        "axes.labelsize": 10,
        "axes.grid": True,
        "grid.alpha": 0.3,
        "figure.dpi": 160,
    })

    # --- Figure 1: Controller IM:ReportData RX Latency CDF Comparison (3 Panels) ---
    fig, (ax1, ax2, ax3) = plt.subplots(1, 3, figsize=(19.2, 6.2))
    fig.suptitle(
        "Matter Groupcast over Thread: Impact of DelayReportData (delayMinMs, delayJitterWindowMs) on Controller IM:ReportData Latency",
        fontsize=13,
        fontweight="bold",
        y=0.98,
    )

    for r in runs:
        c_rx = r["raw_data"]["controller_rep_rx_ms"]
        st = r["stats"]["controller_rep_rx_ms"]
        plot_cdf_line(
            ax1,
            c_rx,
            f"{r['short_label']} (P50={st['p50']:.0f} ms, P90={st['p90']:.0f} ms, P99={st['p99']:.0f} ms)",
            r["color"],
            linestyle=r["linestyle"],
        )

        post_rx = r["raw_data"]["post_deferral_ctrl_rx_ms"]
        pst = r["stats"]["post_deferral_ctrl_rx_ms"]
        plot_cdf_line(
            ax2,
            post_rx,
            f"{r['short_label']} (P50={pst['p50']:.0f} ms, P90={pst['p90']:.0f} ms, P99={pst['p99']:.0f} ms)",
            r["color"],
            linestyle=r["linestyle"],
        )

        q_delay = r["raw_data"]["router_queue_delay_ms"]
        qst = r["stats"]["router_queue_delay_ms"]
        plot_cdf_line(
            ax3,
            q_delay,
            f"{r['short_label']} (P50={qst['p50']:.1f} ms, P90={qst['p90']:.1f} ms, P95={qst['p95']:.1f} ms)",
            r["color"],
            linestyle=r["linestyle"],
        )

    ax1.set_title("A. Controller IM:ReportData RX Latency (From Command TX)")
    ax1.set_xlabel("Controller IM:ReportData RX Latency from Command TX (ms)")
    ax1.set_ylabel("CDF (Cumulative Probability)")
    ax1.set_xlim(-20, 2600)
    ax1.set_ylim(-0.02, 1.03)
    ax1.set_yticks(np.linspace(0, 1.0, 11))
    ax1.axhline(0.50, color="gray", linestyle=":", linewidth=0.9, alpha=0.6)
    ax1.axhline(0.90, color="gray", linestyle=":", linewidth=0.9, alpha=0.6)
    ax1.axhline(0.99, color="gray", linestyle=":", linewidth=0.9, alpha=0.6)
    ax1.legend(loc="lower right", frameon=True, fontsize=8.2)

    ax2.set_title("B. Post-Deferral Controller RX Latency (Controller RX - delayMinMs)")
    ax2.set_xlabel("Latency Relative to Deferral Window Start: t_RX - delayMinMs (ms)")
    ax2.set_ylabel("CDF (Cumulative Probability)")
    ax2.set_xlim(-20, 2300)
    ax2.set_ylim(-0.02, 1.03)
    ax2.set_yticks(np.linspace(0, 1.0, 11))
    ax2.axhline(0.50, color="gray", linestyle=":", linewidth=0.9, alpha=0.6)
    ax2.axhline(0.90, color="gray", linestyle=":", linewidth=0.9, alpha=0.6)
    ax2.axhline(0.99, color="gray", linestyle=":", linewidth=0.9, alpha=0.6)
    ax2.legend(loc="lower right", frameon=True, fontsize=8.2)

    ax3.set_title("C. Unicast Hop-2 Router Queueing Delay (OTBR RX - Node TX)")
    ax3.set_xlabel("Router Forwarding & Queueing Delay on Unicast Return Path (ms)")
    ax3.set_ylabel("CDF (Cumulative Probability)")
    ax3.set_xlim(-5, 400)
    ax3.set_ylim(-0.02, 1.03)
    ax3.set_yticks(np.linspace(0, 1.0, 11))
    ax3.axhline(0.50, color="gray", linestyle=":", linewidth=0.9, alpha=0.6)
    ax3.axhline(0.90, color="gray", linestyle=":", linewidth=0.9, alpha=0.6)
    ax3.axhline(0.95, color="gray", linestyle=":", linewidth=0.9, alpha=0.6)
    ax3.legend(loc="lower right", frameon=True, fontsize=8.2)

    plt.tight_layout(rect=[0, 0, 1, 0.94])
    fig1_path = os.path.join(out_dir, "cdf_delay_report_controller_rx.png")
    plt.savefig(fig1_path)
    plt.close(fig)

    # --- Figure 2: Packet Drops & Mesh Congestion Breakdown Across Runs (3 Panels) ---
    fig, (ax1, ax2, ax3) = plt.subplots(1, 3, figsize=(19.5, 6.4))
    fig.suptitle(
        "Matter Groupcast over Thread: Packet Drops, 802.15.4 MAC Collisions, and Command Delivery Across DelayReportData Configs (50 Cmds / Run)",
        fontsize=12.8,
        fontweight="bold",
        y=0.98,
    )

    x_idx = np.arange(len(runs))
    x_labels = [r["short_label"].replace(" (", "\n(") for r in runs]

    # Panel A: Unicast Return-Path Packet Drops, MRP Retransmissions, and Missed Reports/Commands (Total in 50-Command Run)
    uni_drops = np.array([r["drops_total"]["unicast_congestion_pkt_drops"] for r in runs], dtype=float)
    mrp_retries = np.array([r["drops_total"]["mrp_retransmissions"] for r in runs], dtype=float)
    missed_reps = np.array([r["drops_total"]["missed_controller_rep_rx"] for r in runs], dtype=float)
    missed_cmds = np.array([r["drops_total"]["missed_matter_cmd_rx"] for r in runs], dtype=float)

    bw1 = 0.22
    b_uni = ax1.bar(
        x_idx - 1.5 * bw1,
        uni_drops,
        bw1,
        label="Dropped Unicast Packets (16-Try MAC Exhaustion)",
        color="#d93025",
    )
    b_mrp = ax1.bar(
        x_idx - 0.5 * bw1,
        mrp_retries,
        bw1,
        label="MRP Unicast Retransmissions (Extra ReportData TX)",
        color="#f29900",
    )
    b_mrep = ax1.bar(
        x_idx + 0.5 * bw1,
        missed_reps,
        bw1,
        label="Missed Controller IM:ReportData (Out of 1,000)",
        color="#9334e6",
    )
    b_mcmd = ax1.bar(
        x_idx + 1.5 * bw1,
        missed_cmds,
        bw1,
        label="Missed Node Group Commands (Out of 1,000)",
        color="#1a73e8",
    )

    for bars in (b_uni, b_mrp, b_mrep, b_mcmd):
        for bar in bars:
            h = bar.get_height()
            ax1.text(
                bar.get_x() + bar.get_width() / 2.0,
                h + 2.5,
                f"{int(round(h))}",
                ha="center",
                va="bottom",
                fontsize=7.5,
                fontweight="bold" if h > 0 else "normal",
            )

    ax1.set_title("A. Unicast Report Drops, MRP Retries & Missed Deliveries (Per Run)")
    ax1.set_xticks(x_idx)
    ax1.set_xticklabels(x_labels, fontsize=8.5)
    ax1.set_ylabel("Total Packet Drops / Retries / Missed Events (50 Group Cmds)")
    ax1.set_ylim(0, max(uni_drops.max(), mrp_retries.max(), missed_reps.max()) * 1.32)
    ax1.legend(loc="upper right", frameon=True, fontsize=7.8)

    # Panel B: 802.15.4 PHY Collisions, MAC Retries, and MPL 2-Frag Broadcast Reassembly Drops (Total per Run)
    fcs_tot = np.array([r["drops_total"]["mac_fcs_err"] for r in runs], dtype=float)
    noack_tot = np.array([r["drops_total"]["mac_tx_noack"] for r in runs], dtype=float)
    cca_tot = np.array([r["drops_total"]["mac_tx_cca_fail"] for r in runs], dtype=float)
    mplfrag_tot = np.array([r["drops_total"]["mpl_reassembly_frag_drops"] for r in runs], dtype=float)

    bw2 = 0.20
    b1 = ax2.bar(x_idx - 1.5 * bw2, fcs_tot, bw2, label="PHY Frame Collisions (error:FcsErr)", color="#d93025")
    b2 = ax2.bar(x_idx - 0.5 * bw2, noack_tot, bw2, label="MAC Unacked TX Attempts (error:NoAck)", color="#f29900")
    b3 = ax2.bar(x_idx + 0.5 * bw2, cca_tot, bw2, label="CSMA/CA CCA Busy (ChannelAccessFailure)", color="#1a73e8")
    b4 = ax2.bar(
        x_idx + 1.5 * bw2,
        mplfrag_tot,
        bw2,
        label="MPL 2-Frag Broadcast Reassembly Drops (dst:0xffff)",
        color="#1e8e3e",
    )

    for bars in (b1, b2, b3, b4):
        for bar in bars:
            h = bar.get_height()
            ax2.text(
                bar.get_x() + bar.get_width() / 2.0,
                h + 90.0,
                f"{int(round(h))}",
                ha="center",
                va="bottom",
                fontsize=6.8,
                rotation=25 if h >= 1000 else 0,
            )

    ax2.set_title("B. Total 802.15.4 MAC Failures & MPL Frag Drops (Per Run)")
    ax2.set_xticks(x_idx)
    ax2.set_xticklabels(x_labels, fontsize=8.5)
    ax2.set_ylabel("Total Events Across All 25 Mesh Nodes (50 Group Cmds)")
    ax2.set_ylim(0, max(cca_tot.max(), noack_tot.max(), fcs_tot.max()) * 1.32)
    ax2.legend(loc="upper right", frameon=True, fontsize=7.8)

    # Panel C: Group Command Reception (First CmdRX) CDF across configurations
    for r in runs:
        cmd_rx = r["raw_data"]["matter_first_rx_ms"]
        cst = r["stats"]["matter_first_rx_ms"]
        deliv = r["cmd_rx_delivery_pct"]
        plot_cdf_line(
            ax3,
            cmd_rx,
            f"{r['short_label']} (Deliv={deliv:.1f}%, P50={cst['p50']:.1f} ms, P90={cst['p90']:.1f} ms)",
            r["color"],
            linestyle=r["linestyle"],
        )

    ax3.set_title("C. Matter Node Group Command Reception (First CmdRX) CDF")
    ax3.set_xlabel("Time to Receive Group Command at Matter Nodes from OTBR TX (ms)")
    ax3.set_ylabel("CDF (Cumulative Probability)")
    ax3.set_xlim(-5, 300)
    ax3.set_ylim(-0.02, 1.03)
    ax3.set_yticks(np.linspace(0, 1.0, 11))
    ax3.axhline(0.50, color="gray", linestyle=":", linewidth=0.9, alpha=0.6)
    ax3.axhline(0.90, color="gray", linestyle=":", linewidth=0.9, alpha=0.6)
    ax3.axhline(0.99, color="gray", linestyle=":", linewidth=0.9, alpha=0.6)
    ax3.legend(loc="lower right", frameon=True, fontsize=8.0)

    plt.tight_layout(rect=[0, 0, 1, 0.94])
    fig2_path = os.path.join(out_dir, "delay_report_packet_drops_comparison.png")
    plt.savefig(fig2_path)
    plt.close(fig)

    # Save summary JSON without raw arrays
    summary_out = []
    for r in runs:
        rcopy = dict(r)
        rcopy.pop("raw_data", None)
        summary_out.append(rcopy)

    stats_path = os.path.join(out_dir, "delay_report_sweep_stats.json")
    with open(stats_path, "w", encoding="utf-8") as f:
        json.dump(summary_out, f, indent=2)
    print(f"Saved plots and stats to {out_dir}")


def find_latest_run_by_tag(sim_logs_dir: str, tag_suffix: str) -> Optional[str]:
    matches = sorted(glob.glob(os.path.join(sim_logs_dir, f"*_{tag_suffix}")))
    return matches[-1] if matches else None


def main() -> int:
    repo_root = Path(__file__).resolve().parents[2]
    sim_logs_dir = (
        sys.argv[1]
        if len(sys.argv) > 1
        else str(repo_root / "sim_logs")
    )
    out_dir = (
        sys.argv[2]
        if len(sys.argv) > 2
        else sim_logs_dir
    )

    specs = [
        ("25cycles_delay0_jitter0", "Baseline: No DelayReportData (0 ms, 0 ms)", "Baseline (0, 0)", "#d93025", "--"),
        ("25cycles_delay0_jitter500", "DelayReportData (min=0 ms, jitter=500 ms)", "Delay (0, 500)", "#f29900", "-"),
        ("25cycles_delay150_jitter300", "DelayReportData (min=150 ms, jitter=300 ms)", "Delay (150, 300)", "#9334e6", "-"),
        ("25cycles_delay300_jitter500", "DelayReportData (min=300 ms, jitter=500 ms)", "Delay (300, 500)", "#1a73e8", "-"),
        ("25cycles_delay300_jitter1000", "DelayReportData (min=300 ms, jitter=1000 ms)", "Delay (300, 1000)", "#1e8e3e", "-"),
        ("25cycles_delay300_jitter2000", "DelayReportData (min=300 ms, jitter=2000 ms)", "Delay (300, 2000)", "#007b83", "-"),
    ]

    runs: List[Dict[str, Any]] = []
    for tag, full_label, short_label, color, ls in specs:
        rdir = find_latest_run_by_tag(sim_logs_dir, tag)
        if rdir and os.path.isfile(os.path.join(rdir, "manifest.json")):
            print(f"Parsing {short_label}: {rdir}")
            runs.append(parse_single_run(rdir, full_label, short_label, color, ls))
        else:
            print(f"Skipping {short_label} (not yet completed: *_{tag})")

    if runs:
        generate_comparison_plots_and_stats(runs, out_dir)
    return 0


if __name__ == "__main__":
    sys.exit(main())
