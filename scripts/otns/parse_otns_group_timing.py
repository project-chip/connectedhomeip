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
Parse an archived OTNS simulation run directory (`sim_logs/<run_id>` or `sim_logs/latest`)
and produce a per-node timing breakdown for Group ON / OFF commands and IM:ReportData updates
across one or more cycles.
"""

import argparse
import io
import json
import os
import re
import sys
from typing import Any, Dict, List, Optional, Tuple


def analyze_run_dir(run_dir: str, write_report: bool = True) -> str:
    """Analyze an archived simulation run directory and return the formatted timing breakdown."""
    manifest_path = os.path.join(run_dir, "manifest.json")
    if not os.path.isfile(manifest_path):
        raise FileNotFoundError(f"manifest.json not found in {run_dir}")

    with open(manifest_path, "r", encoding="utf-8") as f:
        manifest: Dict[str, Any] = json.load(f)

    nodes_meta: List[Dict[str, Any]] = manifest.get("nodes", [])
    otbr_log_path = os.path.join(run_dir, "nodes", "0_1.log")
    if not os.path.isfile(otbr_log_path):
        raise FileNotFoundError(f"OTBR log not found at {otbr_log_path}")

    with open(otbr_log_path, "r", encoding="utf-8", errors="replace") as f:
        otbr_lines = [line.rstrip("\n") for line in f]

    # Locate OTBR MPL multicast transmissions (len:162+ to ff03::fc).
    # Trickle repeats of the same command occur within ~200 ms; distinct commands are > 1.5s apart.
    otbr_mcast_tx: List[Tuple[int, str]] = []
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
                    if not otbr_mcast_tx or (sim_us - otbr_mcast_tx[-1][0]) > 1_500_000:
                        otbr_mcast_tx.append((sim_us, m.group(2)))

    ct_log_path = os.path.join(run_dir, "controller", "chip_tool_interactive.log")
    has_interactive = os.path.isfile(ct_log_path)
    tx_events: List[Tuple[str, float, float, int]] = []
    node_ips: Dict[int, str] = {}
    ct_reports_by_idx: Dict[int, Dict[int, Dict[str, Any]]] = {}

    if has_interactive:
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
        val_re = re.compile(
            r"\[(?P<ts>\d+\.\d+)\].*\bOnOff:\s*(?P<val>TRUE|FALSE)\b",
            re.IGNORECASE,
        )

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
                        if cur_node not in ct_reports_by_idx[idx_ev]:
                            ct_reports_by_idx[idx_ev][cur_node] = {
                                "rx_ts": cur_ts,
                                "val": val,
                                "delta_ms": (cur_ts - wpan_ts) * 1000.0,
                            }

    num_cmds = len(otbr_mcast_tx)
    cmd_windows: List[Tuple[int, int, str, Optional[float]]] = []
    for k in range(num_cmds):
        t0 = otbr_mcast_tx[k][0]
        t1 = (otbr_mcast_tx[k + 1][0] - 20_000) if k + 1 < num_cmds else 999_999_999_999
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

    out = io.StringIO()
    out.write(f"Run ID:       {manifest.get('run_id')}\n")
    out.write(f"Started At:   {manifest.get('started_at')}\n")
    out.write(f"Completed At: {manifest.get('completed_at')}\n")
    out.write(f"State Reused: {manifest.get('state_reused')}\n")
    out.write(f"Wildcard Sub: {manifest.get('subscribe_wildcard')}\n")
    out.write(f"Group Cycles: {manifest.get('group_cycles', max(1, num_cmds // 2))}\n\n")

    node_records: Dict[int, Dict[str, Any]] = {}
    for nmeta in nodes_meta:
        otns_id = int(nmeta["otns_id"])
        nlog_path = os.path.join(run_dir, nmeta["raw_log"])
        nlines: List[str] = []
        if os.path.isfile(nlog_path):
            with open(nlog_path, "r", encoding="utf-8", errors="replace") as f:
                nlines = [line.rstrip("\n") for line in f]

        cmd_rx_by_k: Dict[int, List[Tuple[int, str, str, str]]] = {k: [] for k in range(num_cmds)}
        rep_tx_by_k: Dict[int, List[Tuple[int, str, str]]] = {k: [] for k in range(num_cmds)}

        for line in nlines:
            if "Received IPv6 HopOpts msg, len:" in line:
                m_len = re.search(r"len:(\d+)", line)
                if not m_len or int(m_len.group(1)) < 160:
                    continue
                m = re.match(
                    r"\s*(\d+)\s+(\S+).*from:(0x[0-9a-fA-F]+).*rss:([-\d\.]+)",
                    line,
                )
                if m:
                    sim_us = int(m.group(1))
                    ntime = m.group(2)
                    sender = m.group(3)
                    rss = m.group(4)
                    for k, (t0, t1, _phase, _ct_t0) in enumerate(cmd_windows):
                        if (t0 - 5000) <= sim_us < t1:
                            cmd_rx_by_k[k].append((sim_us, ntime, sender, rss))
                            break
            elif "Sent IPv6 UDP msg, len:117," in line:
                m = re.match(r"\s*(\d+)\s+(\S+).*to:(0x[0-9a-fA-F]+)", line)
                if m:
                    sim_us = int(m.group(1))
                    ntime = m.group(2)
                    nhop = m.group(3)
                    for k, (t0, t1, _phase, _ct_t0) in enumerate(cmd_windows):
                        if t0 <= sim_us < t1:
                            rep_tx_by_k[k].append((sim_us, ntime, nhop))
                            break

        node_records[otns_id] = {
            "meta": nmeta,
            "cmd_rx": cmd_rx_by_k,
            "rep_tx": rep_tx_by_k,
        }

    for k, (sim_t0, _sim_t1, phase, ct_t0) in enumerate(cmd_windows):
        ct_str_hdr = f"{ct_t0:.3f}" if ct_t0 is not None else "N/A"
        cycle_num = (k // 2) + 1
        cycle_prefix = f"CYCLE {cycle_num} " if num_cmds > 2 else ""
        out.write(f"=== {cycle_prefix}GROUP {phase} (t0_sim = {sim_t0} us, t0_controller = {ct_str_hdr}) ===\n")
        for otns_id in sorted(node_records.keys()):
            rec = node_records[otns_id]
            nmeta = rec["meta"]
            mid = nmeta.get("matter_node_id")
            mid_str = str(mid) if mid is not None else "-"
            rloc_str = nmeta.get("rloc16") or "?"
            rx_list = rec["cmd_rx"][k]
            rep_tx_list = rec["rep_tx"][k]

            if otns_id == 1:
                rx_str = f"{sim_t0} (+0.000 ms, TX)"
            elif rx_list:
                first_us, _ntime, sender, rss = rx_list[0]
                rx_str = (
                    f"{first_us} (+{(first_us - sim_t0) / 1000.0:.3f} ms) "
                    f"from {sender} ({rss}dBm, {len(rx_list)}x)"
                )
            else:
                rx_str = "DROPPED"

            if mid is not None:
                tx_str = (
                    f"{rep_tx_list[0][0]} (+{(rep_tx_list[0][0] - sim_t0) / 1000.0:.3f} ms, {len(rep_tx_list)}x)"
                    if rep_tx_list
                    else "-"
                )
                otbr_rx = otbr_report_rx[k].get(mid)
                otbr_str = f"{otbr_rx[0]} (+{otbr_rx[1]:.3f} ms)" if otbr_rx else "-"
                ct_rep = ct_reports_by_idx.get(k, {}).get(mid)
                ct_str = (
                    f"{ct_rep['rx_ts']:.3f} (+{ct_rep['delta_ms']:.1f} ms, {ct_rep['val']})"
                    if ct_rep
                    else "-"
                )
            else:
                tx_str = "-"
                otbr_str = "-"
                ct_str = "-"

            out.write(
                f"Node {otns_id:2d} (Matter {mid_str:>3s}, {rloc_str:>6s}): "
                f"CmdRX={rx_str} | NodeRepTX={tx_str} | OTBRRepRX={otbr_str} | ControllerRepRX={ct_str}\n"
            )
        out.write("\n")

    report_text = out.getvalue()
    if write_report:
        report_path = os.path.join(run_dir, "timing_breakdown.txt")
        with open(report_path, "w", encoding="utf-8") as f:
            f.write(report_text)
    return report_text


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Analyze timing breakdown of an archived OTNS Matter simulation run"
    )
    parser.add_argument(
        "run_dir",
        nargs="?",
        default="/workspace/sim_logs/latest",
        help="Path to archived simulation run directory (default: /workspace/sim_logs/latest)",
    )
    args = parser.parse_args()
    text = analyze_run_dir(os.path.realpath(args.run_dir), write_report=True)
    print(text, end="")
    return 0


if __name__ == "__main__":
    sys.exit(main())
