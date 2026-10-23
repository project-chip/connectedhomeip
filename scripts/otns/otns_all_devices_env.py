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
OTNS + Simulated Thread Border Router (OTBR) + Matter all-devices-app + chip-tool environment.

Supports both single-node CLI invocation and multi-node declarative JSON configuration
files (including Groupcast 0x0065 group configuration and multicast verification).

Runs inside the framichel/otns-matter container with /workspace mounted to the
connectedhomeip repository.
"""

import argparse
from dataclasses import asdict, dataclass, field
from datetime import datetime, timezone
from zoneinfo import ZoneInfo
import glob
import json
import os
import re
import shutil
import subprocess
import sys
import threading
import time
from typing import Any, Dict, List, Optional, Tuple

OTNS_PYLIBS_DIR = "/home/otns/ot-ns/pylibs"
if os.path.isdir(OTNS_PYLIBS_DIR) and OTNS_PYLIBS_DIR not in sys.path:
    sys.path.insert(0, OTNS_PYLIBS_DIR)

for extra_bin_dir in ["/home/otns/go/bin", "/usr/local/go/bin", "/usr/sbin", "/sbin"]:
    if os.path.isdir(extra_bin_dir) and extra_bin_dir not in os.environ.get("PATH", "").split(os.pathsep):
        os.environ["PATH"] = f"{extra_bin_dir}{os.pathsep}{os.environ.get('PATH', '')}"

from otns.cli import OTNS

OT_VERSIONS_DIR = "/home/otns/ot-ns/ot-rfsim/ot-versions"
OT_TMP_DIR = os.path.join(OT_VERSIONS_DIR, "tmp")
WRAPPER_DIR = "/tmp/otns_matter_wrappers"
SAVED_STATE_DIR = "/tmp/saved_commissioned_state"
INTERACTIVE_LOG_PATH = "/tmp/chip_tool_interactive.log"
CHIP_TOOL_COMMANDS_LOG_PATH = "/tmp/chip_tool_commands.log"
DEFAULT_SIM_LOGS_DIR = "/workspace/sim_logs"
DEFAULT_ALL_DEVICES_BIN = (
    "/workspace/out/linux-x64-all-devices-no-wifi-no-ble-openthread-endpoint-otns/all-devices-app"
)
DEFAULT_CHIP_TOOL_BIN = "/workspace/out/linux-x64-chip-tool-no-ble/chip-tool"


@dataclass
class GroupSpec:
    group_id: str
    name: str
    keyset_id: str
    key: str
    use_auxiliary_acl: bool = True
    mcast_addr_policy: int = 0  # 0 = IanaAddr (ff05::fa), 1 = PerGroup (ff35:0040:fd...)
    endpoints: List[int] = field(default_factory=lambda: [1])

    @property
    def group_id_int(self) -> int:
        return int(self.group_id, 0)

    @property
    def multicast_node_id_hex(self) -> str:
        return f"0xffffffffffff{self.group_id_int:04x}"


@dataclass
class MatterNodeSpec:
    node_id: int
    discriminator: int
    passcode: int
    devices: List[str]
    x: int
    y: int
    radio_range: Optional[int] = 250
    groupcast: bool = True
    router_eligible: bool = False
    groups: List[str] = field(default_factory=list)
    otns_id: Optional[int] = None
    wrapper_path: Optional[str] = None
    log_path: Optional[str] = None


@dataclass
class EnvironmentConfig:
    otbr_x: int = 450
    otbr_y: int = 250
    otbr_radio_range: Optional[int] = 250
    batch_size: int = 4
    routers: List[Dict[str, int]] = field(default_factory=list)
    groups: List[GroupSpec] = field(default_factory=list)
    matter_nodes: List[MatterNodeSpec] = field(default_factory=list)


def load_environment_config(args: argparse.Namespace) -> EnvironmentConfig:
    """Build EnvironmentConfig either from --config JSON file or from CLI arguments."""
    if not args.config:
        routers = [
            {"x": 375, "y": 180 + i * 120, "radio_range": 250}
            for i in range(args.num_routers)
        ]
        node = MatterNodeSpec(
            node_id=args.node_id,
            discriminator=args.discriminator,
            passcode=args.passcode,
            devices=[args.device],
            x=300,
            y=250,
            radio_range=250,
            groupcast=True,
            groups=[],
        )
        return EnvironmentConfig(
            otbr_x=450,
            otbr_y=250,
            otbr_radio_range=250,
            routers=routers,
            groups=[],
            matter_nodes=[node],
        )

    with open(args.config, "r", encoding="utf-8") as f:
        raw: Dict[str, Any] = json.load(f)

    otbr_cfg = raw.get("otbr", {})
    otbr_x = int(otbr_cfg.get("x", 400))
    otbr_y = int(otbr_cfg.get("y", 300))
    otbr_radio_range = otbr_cfg.get("radio_range", 250)

    routers = list(raw.get("routers", []))

    groups: List[GroupSpec] = []
    for g in raw.get("groups", []):
        groups.append(
            GroupSpec(
                group_id=str(g["group_id"]),
                name=str(g.get("name", f"Group_{g['group_id']}")),
                keyset_id=str(g.get("keyset_id", "0x01a1")),
                key=str(g.get("key", "b0b1b2b3b4b5b6b7b8b9babbbcbdbebf")),
                use_auxiliary_acl=bool(g.get("use_auxiliary_acl", True)),
                mcast_addr_policy=int(g.get("mcast_addr_policy", 0)),
                endpoints=[int(ep) for ep in g.get("endpoints", [1])],
            )
        )

    matter_nodes: List[MatterNodeSpec] = []
    batch_size = 4
    matter_cfg = raw.get("matter_nodes", {})
    if isinstance(matter_cfg, dict) and "count" in matter_cfg:
        count = int(matter_cfg["count"])
        batch_size = int(matter_cfg.get("batch_size", 4))
        base_node_id = int(matter_cfg.get("base_node_id", 101))
        base_discriminator = int(matter_cfg.get("base_discriminator", 3840))
        passcode = int(matter_cfg.get("passcode", 20202021))
        groupcast = bool(matter_cfg.get("groupcast", True))
        router_eligible = bool(matter_cfg.get("router_eligible", False))
        devices = [str(d) for d in matter_cfg.get("devices", ["on-off-light:1"])]
        node_groups = [str(gid) for gid in matter_cfg.get("groups", [])]

        layout = matter_cfg.get("layout", {})
        columns = int(layout.get("columns", 5))
        start_x = int(layout.get("start_x", 260))
        start_y = int(layout.get("start_y", 195))
        spacing_x = int(layout.get("spacing_x", 70))
        spacing_y = int(layout.get("spacing_y", 70))
        radio_range = layout.get("radio_range", 250)

        for idx in range(count):
            col = idx % columns
            row = idx // columns
            matter_nodes.append(
                MatterNodeSpec(
                    node_id=base_node_id + idx,
                    discriminator=base_discriminator + idx,
                    passcode=passcode,
                    devices=list(devices),
                    x=start_x + col * spacing_x,
                    y=start_y + row * spacing_y,
                    radio_range=int(radio_range) if radio_range is not None else None,
                    groupcast=groupcast,
                    router_eligible=router_eligible,
                    groups=list(node_groups),
                )
            )
    elif isinstance(matter_cfg, list):
        for idx, item in enumerate(matter_cfg):
            matter_nodes.append(
                MatterNodeSpec(
                    node_id=int(item.get("node_id", 101 + idx)),
                    discriminator=int(item.get("discriminator", 3840 + idx)),
                    passcode=int(item.get("passcode", 20202021)),
                    devices=[str(d) for d in item.get("devices", ["on-off-light:1"])],
                    x=int(item.get("x", 260 + (idx % 5) * 70)),
                    y=int(item.get("y", 195 + (idx // 5) * 70)),
                    radio_range=item.get("radio_range", 250),
                    groupcast=bool(item.get("groupcast", True)),
                    router_eligible=bool(item.get("router_eligible", False)),
                    groups=[str(gid) for gid in item.get("groups", [])],
                )
            )

    return EnvironmentConfig(
        otbr_x=otbr_x,
        otbr_y=otbr_y,
        otbr_radio_range=int(otbr_radio_range) if otbr_radio_range is not None else None,
        batch_size=batch_size,
        routers=routers,
        groups=groups,
        matter_nodes=matter_nodes,
    )


def detect_backbone_interface() -> str:
    """Detect the first non-loopback, non-wpan interface that is up."""
    try:
        result = subprocess.run(
            ["ip", "-o", "link", "show", "up"],
            capture_output=True,
            text=True,
            check=True,
        )
        for line in result.stdout.strip().splitlines():
            parts = line.split(": ")
            if len(parts) >= 2:
                iface = parts[1].split("@")[0]
                if iface != "lo" and not iface.startswith("wpan"):
                    return iface
    except (subprocess.CalledProcessError, FileNotFoundError):
        pass
    return "eth0"


def ensure_otns_patches() -> None:
    """Ensure OTNS simulation/node.go and dispatcher/dispatcher.go have Matter & real-time sync fixes."""
    otns_root = "/home/otns/ot-ns"
    node_go_path = os.path.join(otns_root, "simulation/node.go")
    disp_go_path = os.path.join(otns_root, "dispatcher/dispatcher.go")
    if not (os.path.isfile(node_go_path) and os.path.isfile(disp_go_path)):
        return

    rebuilt_needed = False

    with open(node_go_path, "r", encoding="utf-8") as f:
        node_src = f.read()
    if "cfg.Type == MATTER" not in node_src:
        node_src = node_src.replace(
            'if cfg.IsRcp {\n\t\t\tnode.uartType = nodeUartTypeRealTime\n\t\t\tgo node.lineReaderStdOut(node.pipeOut) // real-UART reader for Posix host CLI output and logging\n\t\t}',
            'if cfg.IsRcp {\n\t\t\tnode.uartType = nodeUartTypeRealTime\n\t\t\tgo node.lineReaderStdOut(node.pipeOut) // real-UART reader for Posix host CLI output and logging\n\t\t} else if cfg.Type == MATTER {\n\t\t\tgo node.lineReaderStdErr(node.pipeOut) // drain Matter stdout (CHIP logs) and write to node log\n\t\t}',
        )
        node_src = node_src.replace(
            "if node.cfg.IsRcp && node.cfg.IsBorderRouter {",
            "if node.cfg.IsRcp && node.cfg.IsBorderRouter || node.cfg.Type == MATTER {",
        )
        with open(node_go_path, "w", encoding="utf-8") as f:
            f.write(node_src)
        rebuilt_needed = True

    with open(disp_go_path, "r", encoding="utf-8") as f:
        disp_src = f.read()
    if "// Wait for all currently alive nodes to finish processing" not in disp_src:
        old_block = (
            "\t\t// Determine simulation time target based on real clock time, and process queued events (if any).\n"
            "\t\tvar simTime uint64"
        )
        new_block = (
            "\t\t// Wait for all currently alive nodes to finish processing the current time instant before advancing CurTime.\n"
            "\t\td.RecvEvents()\n"
            "\t\td.syncAliveNodes()\n\n"
            "\t\t// Determine simulation time target based on real clock time, and process queued events (if any).\n"
            "\t\tvar simTime uint64"
        )
        old_proc = """\t// process (if any) all queued events, that happen at exactly procUntilTime
\tprocUntilTime := nextEventTime
\tfor nextEventTime <= procUntilTime {
\t\tif nextAlarmTime <= nextSendTime {
\t\t\t// process next alarm
\t\t\tnextAlarm := d.alarmMgr.NextAlarm()
\t\t\tlogger.AssertNotNil(nextAlarm)
\t\t\tif node := d.nodes[nextAlarm.NodeId]; node != nil {
\t\t\t\td.advanceNodeTime(node, nextAlarmTime, false)
\t\t\t}
\t\t} else {
\t\t\t// process next event from the queue
\t\t\tevt := d.eventQueue.PopNext()
\t\t\tlogger.AssertTrue(evt.Timestamp == nextEventTime)
\t\t\tlogger.AssertTrue(nextAlarmTime <= d.CurTime || nextSendTime <= d.CurTime)
\t\t\td.processEvent(evt)
\t\t}
\t\tnextAlarmTime = d.alarmMgr.NextTimestamp()
\t\tnextSendTime = d.eventQueue.NextTimestamp()
\t\tnextEventTime = min(nextAlarmTime, nextSendTime)
\t}

\treturn nextEventTime"""
        new_proc = """\t// process (if any) all queued events, that happen at exactly procUntilTime
\tprocUntilTime := nextEventTime
\tfor nextEventTime <= procUntilTime {
\t\tfor nextEventTime <= procUntilTime {
\t\t\tif nextAlarmTime <= nextSendTime {
\t\t\t\t// process next alarm
\t\t\t\tnextAlarm := d.alarmMgr.NextAlarm()
\t\t\t\tlogger.AssertNotNil(nextAlarm)
\t\t\t\tif node := d.nodes[nextAlarm.NodeId]; node != nil {
\t\t\t\t\td.advanceNodeTime(node, nextAlarmTime, false)
\t\t\t\t}
\t\t\t} else {
\t\t\t\t// process next event from the queue
\t\t\t\tevt := d.eventQueue.PopNext()
\t\t\t\tlogger.AssertTrue(evt.Timestamp == nextEventTime)
\t\t\t\tlogger.AssertTrue(nextAlarmTime <= d.CurTime || nextSendTime <= d.CurTime)
\t\t\t\td.processEvent(evt)
\t\t\t}
\t\t\tnextAlarmTime = d.alarmMgr.NextTimestamp()
\t\t\tnextSendTime = d.eventQueue.NextTimestamp()
\t\t\tnextEventTime = min(nextAlarmTime, nextSendTime)
\t\t}
\t\tif len(d.aliveNodes) == 0 {
\t\t\tbreak
\t\t}
\t\td.RecvEvents()
\t\td.syncAliveNodes()
\t\tnextAlarmTime = d.alarmMgr.NextTimestamp()
\t\tnextSendTime = d.eventQueue.NextTimestamp()
\t\tnextEventTime = min(nextAlarmTime, nextSendTime)
\t}

\treturn nextEventTime"""
        if old_block in disp_src:
            disp_src = disp_src.replace(old_block, new_block, 1)
            if old_proc in disp_src:
                disp_src = disp_src.replace(old_proc, new_proc, 1)
            with open(disp_go_path, "w", encoding="utf-8") as f:
                f.write(disp_src)
            rebuilt_needed = True

    if rebuilt_needed:
        print("Rebuilding /home/otns/go/bin/otns with Matter & real-time synchronization fixes...")
        subprocess.run(
            ["su", "-", "otns", "-c", "export PATH=$PATH:/usr/local/go/bin:~/go/bin && cd /home/otns/ot-ns && go install ./cmd/otns"],
            check=True,
        )


def _strip_persisted_subscriptions(kvs_path: str) -> None:
    """Remove persisted SubscriptionResumptionStorage keys (g/su/*, g/sum) from a Matter KVS file."""
    if not os.path.isfile(kvs_path):
        return
    with open(kvs_path, "r", encoding="utf-8", errors="ignore") as f:
        lines = f.readlines()
    filtered = [line for line in lines if not (line.startswith("g/su/") or line.startswith("g/sum="))]
    if len(filtered) != len(lines):
        with open(kvs_path, "w", encoding="utf-8") as f:
            f.writelines(filtered)


def save_commissioned_state(env_cfg: EnvironmentConfig) -> None:
    """Save commissioned KVS files and chip-tool storage to SAVED_STATE_DIR for future --reuse-state runs."""
    os.makedirs(SAVED_STATE_DIR, exist_ok=True)
    for node in env_cfg.matter_nodes:
        src = f"/tmp/chip_kvs_otns_{node.node_id}"
        if os.path.isfile(src):
            _strip_persisted_subscriptions(src)
            dst = os.path.join(SAVED_STATE_DIR, os.path.basename(src))
            shutil.copy2(src, dst)
            _strip_persisted_subscriptions(dst)
    for ini_path in glob.glob("/tmp/chip_*.ini"):
        if os.path.isfile(ini_path):
            shutil.copy2(ini_path, os.path.join(SAVED_STATE_DIR, os.path.basename(ini_path)))
    if os.path.isdir("/tmp/chip_tool_kvs"):
        dst_kvs = os.path.join(SAVED_STATE_DIR, "chip_tool_kvs")
        os.makedirs(dst_kvs, exist_ok=True)
        for item in os.listdir("/tmp/chip_tool_kvs"):
            s = os.path.join("/tmp/chip_tool_kvs", item)
            d = os.path.join(dst_kvs, item)
            if os.path.isfile(s):
                shutil.copy2(s, d)


def restore_commissioned_state(env_cfg: EnvironmentConfig) -> bool:
    """Restore commissioned KVS files from SAVED_STATE_DIR if available; return True if all nodes have KVS."""
    if os.path.isdir(SAVED_STATE_DIR):
        for item in os.listdir(SAVED_STATE_DIR):
            s = os.path.join(SAVED_STATE_DIR, item)
            d = os.path.join("/tmp", item)
            if os.path.isdir(s):
                os.makedirs(d, exist_ok=True)
                for sub in os.listdir(s):
                    sub_s = os.path.join(s, sub)
                    sub_d = os.path.join(d, sub)
                    if os.path.isfile(sub_s):
                        shutil.copy2(sub_s, sub_d)
            elif os.path.isfile(s):
                # Keep the newer chip_counters.ini if /tmp/chip_counters.ini already exists
                if item == "chip_counters.ini" and os.path.isfile(d):
                    continue
                if item.startswith("chip_kvs_otns_"):
                    _strip_persisted_subscriptions(s)
                shutil.copy2(s, d)

    for node in env_cfg.matter_nodes:
        _strip_persisted_subscriptions(f"/tmp/chip_kvs_otns_{node.node_id}")

    return all(os.path.isfile(f"/tmp/chip_kvs_otns_{node.node_id}") for node in env_cfg.matter_nodes)


def prepare_system(
    backbone_if: str,
    all_devices_bin: str,
    env_cfg: EnvironmentConfig,
    reuse_state: bool = False,
) -> bool:
    """Ensure required system services, sysctls, and per-node wrapper scripts are configured.

    Returns True if existing commissioned state is active and ready to reuse.
    """
    ensure_otns_patches()
    # Cleanly stop any leftover processes from previous runs (use -x so we never match python3)
    for proc_name in [
        "otns",
        "ot-matter-node",
        "all-devices-app",
        "otbr-agent",
        "ot-cli-ftd",
        "ot-cli-mtd",
        "ot-rcp",
        "ot-ctl",
        "chip-tool",
    ]:
        subprocess.run(["pkill", "-9", "-x", proc_name], check=False, stderr=subprocess.DEVNULL)
    time.sleep(1.0)

    subprocess.run(
        [
            "sysctl",
            "-w",
            "net.ipv6.conf.all.disable_ipv6=0",
            "net.ipv6.conf.default.disable_ipv6=0",
            f"net.ipv6.conf.{backbone_if}.disable_ipv6=0",
            "net.ipv6.conf.all.forwarding=1",
            "net.ipv6.auto_flowlabels=0",
        ],
        check=False,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )

    state_reused = False
    if reuse_state:
        state_reused = restore_commissioned_state(env_cfg)
        if state_reused:
            print(f"  [reuse-state] Reusing commissioned KVS state for {len(env_cfg.matter_nodes)} nodes.")
        else:
            print("  [reuse-state] Saved state incomplete; falling back to fresh commissioning.")

    # Clean up stale logs and wrappers (and Matter KVS only when not reusing state)
    cleanup_patterns = [
        "/tmp/all_devices_node_*.log",
        "/tmp/otns_matter_wrappers",
        "/tmp/mdnsd.log",
        "/tmp/otbr_stderr.log",
        INTERACTIVE_LOG_PATH,
        CHIP_TOOL_COMMANDS_LOG_PATH,
    ]
    if not state_reused:
        cleanup_patterns.extend(["/tmp/chip_*", "/tmp/chip-*"])

    for pattern in cleanup_patterns:
        for path in glob.glob(pattern):
            try:
                if os.path.isdir(path) and not os.path.islink(path):
                    subprocess.run(["rm", "-rf", path], check=False)
                else:
                    os.remove(path)
            except OSError:
                pass

    # Ensure ot-br.sh line-buffers otbr-agent stdout (OpenThread [OTNS] status pushes)
    # and separates otbrLog stderr ([INFO]-HELPER--) so they never interleave mid-line.
    ot_br_sh_path = os.path.join(OT_VERSIONS_DIR, "ot-br.sh")
    if os.path.isfile(ot_br_sh_path):
        with open(ot_br_sh_path, "r", encoding="utf-8") as f:
            ot_br_sh = f.read()
        old_line = 'sudo otbr-agent -s -d 7 -I ${THREAD_IF_NAME} -B ${BACKBONE_IF_NAME} "${RADIO_URL}" 1>&2 &'
        new_line = (
            'sudo stdbuf -oL otbr-agent -s -d 7 -I ${THREAD_IF_NAME} -B ${BACKBONE_IF_NAME} '
            '"${RADIO_URL}" 1>&2 2>/tmp/otbr_stderr.log &'
        )
        if old_line in ot_br_sh:
            ot_br_sh = ot_br_sh.replace(old_line, new_line)
            with open(ot_br_sh_path, "w", encoding="utf-8") as f:
                f.write(ot_br_sh)
            os.chmod(ot_br_sh_path, 0o755)

    # Ensure dbus-daemon, rsyslogd, and mdnsd are running
    os.makedirs("/tmp/chip_tool_kvs", exist_ok=True)
    os.makedirs(WRAPPER_DIR, exist_ok=True)
    os.makedirs("/run/dbus", exist_ok=True)
    subprocess.run(["dbus-daemon", "--system", "--nopidfile"], check=False, stderr=subprocess.DEVNULL)
    subprocess.run(["rsyslogd"], check=False, stderr=subprocess.DEVNULL)

    # Restart mdnsd so stale mDNS records from prior runs are flushed
    subprocess.run(["pkill", "-9", "-x", "mdnsd"], check=False, stderr=subprocess.DEVNULL)
    time.sleep(0.2)
    with open("/tmp/mdnsd.log", "wb") as logf:
        subprocess.Popen(["mdnsd"], stdout=logf, stderr=logf)
    time.sleep(1.0)

    # Create per-node wrapper scripts
    for node in env_cfg.matter_nodes:
        device_flags = " ".join(f'--device "{dev}"' for dev in node.devices)
        groupcast_flag = "--groupcast" if node.groupcast else ""
        kvs_path = f"/tmp/chip_kvs_otns_{node.node_id}"
        log_path = f"/tmp/all_devices_node_{node.node_id}.log"
        wrapper_path = os.path.join(WRAPPER_DIR, f"ot-matter-node-{node.node_id}.sh")
        wrapper_content = f"""#!/bin/bash
exec -a ot-matter-node {all_devices_bin} \\
  {device_flags} \\
  --discriminator {node.discriminator} \\
  --passcode {node.passcode} \\
  {groupcast_flag} \\
  --KVS "{kvs_path}" \\
  "$@"
"""
        with open(wrapper_path, "w", encoding="utf-8") as f:
            f.write(wrapper_content)
        os.chmod(wrapper_path, 0o755)
        node.wrapper_path = wrapper_path
        node.log_path = log_path

    # Also write default wrapper in OT_VERSIONS_DIR for interactive `add matter` usage
    default_device = env_cfg.matter_nodes[0].devices[0] if env_cfg.matter_nodes else "on-off-light:1"
    default_wrapper_path = os.path.join(OT_VERSIONS_DIR, "ot-matter-node")
    if os.path.islink(default_wrapper_path):
        os.unlink(default_wrapper_path)
    default_wrapper_content = f"""#!/bin/bash
exec -a ot-matter-node {all_devices_bin} \\
  --device "{default_device}" \\
  --groupcast \\
  --KVS "/tmp/chip_kvs_otns_$$" \\
  "$@"
"""
    with open(default_wrapper_path, "w", encoding="utf-8") as f:
        f.write(default_wrapper_content)
    os.chmod(default_wrapper_path, 0o755)
    return state_reused


def configure_border_router_routes(omr_prefix_lines: List[str]) -> None:
    """Configure Linux unicast OMR prefix and Matter IPv6 multicast routes to wpan0."""
    # Extract IPv6 prefix (e.g., fdde:ad00:beef:2::/64) from `br omrprefix` output
    prefix_match = None
    for line in omr_prefix_lines:
        m = re.search(r"([0-9a-fA-F:]+::/\d+)", line)
        if m:
            prefix_match = m.group(1)
            break

    if prefix_match:
        subprocess.run(
            ["ip", "-6", "route", "replace", prefix_match, "dev", "wpan0", "metric", "100"],
            check=False,
            stderr=subprocess.DEVNULL,
        )

    # Route Matter Site-Local multicast addresses (ff05::fa for IanaAddr and ff35::/16 for PerGroup)
    # out wpan0 so otbr-agent encapsulates them into MPL (ff03::fc) across the Thread mesh.
    for mcast_prefix in ["ff05::/16", "ff35::/16"]:
        subprocess.run(
            [
                "ip",
                "-6",
                "route",
                "replace",
                "multicast",
                mcast_prefix,
                "dev",
                "wpan0",
                "table",
                "local",
                "metric",
                "100",
            ],
            check=False,
            stderr=subprocess.DEVNULL,
        )
        subprocess.run(
            ["ip", "-6", "route", "replace", mcast_prefix, "dev", "wpan0", "metric", "100"],
            check=False,
            stderr=subprocess.DEVNULL,
        )


def wait_for_otbr_ready(ns: OTNS, otbr_id: int, timeout_s: int = 35) -> bool:
    """Wait until the OTBR node becomes Leader, SRP server is running, and OMR prefix is active."""
    deadline = time.time() + timeout_s
    while time.time() < deadline:
        try:
            state_lines = ns.node_cmd(otbr_id, "state")
            state = state_lines[0].strip() if state_lines else "unknown"
            if state not in ("leader", "router"):
                print(f"  [otbr] state={state} (forming Thread network...)")
                time.sleep(1.5)
                continue
            srp_lines = ns.node_cmd(otbr_id, "srp server state")
            srp_state = srp_lines[0].strip() if srp_lines else "unknown"
            omr_lines = ns.node_cmd(otbr_id, "br omrprefix")
            configure_border_router_routes(omr_lines)
            print(f"  [otbr] state={state} srp_server={srp_state} omr={omr_lines}")
            if srp_state == "running" and any("::/" in l for l in omr_lines):
                return True
        except Exception as exc:
            print(f"  [otbr] polling exception: {exc}")
        time.sleep(1.5)
    return False


def wait_for_routers_ready(ns: OTNS, router_ids: List[int], timeout_s: int = 35) -> bool:
    """Wait until all dedicated Thread router nodes attach to the mesh."""
    if not router_ids:
        return True
    deadline = time.time() + timeout_s
    while time.time() < deadline:
        attached = 0
        states = {}
        for rid in router_ids:
            try:
                lines = ns.node_cmd(rid, "state")
                st = lines[0].strip() if lines else "unknown"
                states[rid] = st
                if st in ("router", "leader"):
                    attached += 1
            except Exception:
                states[rid] = "error"
        print(f"  [routers] attached={attached}/{len(router_ids)} states={states}")
        if attached == len(router_ids):
            return True
        time.sleep(1.5)
    return False


def wait_for_mesh_and_srp(
    ns: OTNS,
    otbr_id: int,
    matter_nodes: List[MatterNodeSpec],
    timeout_s: int = 120,
) -> bool:
    """Wait until all given Matter nodes attach to Thread and register _matterc._udp via SRP on the OTBR."""
    expected_count = len(matter_nodes)
    start_time = time.time()
    deadline = start_time + timeout_s
    last_nudge = start_time
    while time.time() < deadline:
        try:
            otbr_state_lines = ns.node_cmd(otbr_id, "state")
            otbr_state = otbr_state_lines[0].strip() if otbr_state_lines else "unknown"
            if otbr_state not in ("leader", "router", "child"):
                print(f"  [mesh] otbr(node {otbr_id})={otbr_state} (waiting for OTBR to attach...)")
                time.sleep(2)
                continue

            attached_count = 0
            registered_client_count = 0
            role_counts: Dict[str, int] = {}
            unregistered_nodes: List[MatterNodeSpec] = []
            for node in matter_nodes:
                assert node.otns_id is not None
                state_lines = ns.node_cmd(node.otns_id, "state")
                state = state_lines[0].strip() if state_lines else "unknown"
                role_counts[state] = role_counts.get(state, 0) + 1
                if state in ("leader", "router", "child"):
                    attached_count += 1
                    client_svc = ns.node_cmd(node.otns_id, "srp client service")
                    if any(("state:Registered" in line or "state:Refreshing" in line) for line in client_svc):
                        registered_client_count += 1
                    else:
                        unregistered_nodes.append(node)

            srp_services = ns.node_cmd(otbr_id, "srp server service")
            omr_prefix = ns.node_cmd(otbr_id, "br omrprefix")
            configure_border_router_routes(omr_prefix)

            active_matter_srp = [
                line
                for line in srp_services
                if ("_matterc._udp" in line or "_matter._tcp" in line)
                and ("deleted: false" in line or "deleted:" not in line)
            ]
            print(
                f"  [mesh] otbr={otbr_state} attached={attached_count}/{expected_count} "
                f"roles={role_counts} client_srp={registered_client_count}/{expected_count} "
                f"server_srp={len(active_matter_srp)}/{expected_count}"
            )
            if (
                attached_count == expected_count
                and registered_client_count == expected_count
                and len(active_matter_srp) >= expected_count
            ):
                print(
                    f"  [mesh] All {expected_count} Matter node(s) attached and registered "
                    f"on OTBR SRP server!"
                )
                return True

            # If any attached node's SRP client is stuck in exponential backoff (>30s), nudge it
            now = time.time()
            if unregistered_nodes and (now - last_nudge) >= 30.0:
                last_nudge = now
                for node in unregistered_nodes:
                    assert node.otns_id is not None
                    print(f"  [mesh] Nudging SRP client on node {node.node_id} (otns_id={node.otns_id})...")
                    try:
                        ns.node_cmd(node.otns_id, "srp client stop")
                        ns.node_cmd(node.otns_id, "srp client autostart enable")
                    except Exception:
                        pass
        except Exception as exc:
            print(f"  [mesh] polling exception: {exc}")
        time.sleep(2)
    return False


def run_chip_tool(
    chip_tool_bin: str,
    args: List[str],
    timeout_s: int = 90,
    retries: int = 2,
) -> str:
    """Execute a chip-tool command on the LAN and return stdout (raising on failure after retries)."""
    os.makedirs("/tmp/chip_tool_kvs", exist_ok=True)
    cmd = [chip_tool_bin, *args, "--storage-directory", "/tmp/chip_tool_kvs"]
    last_err: Optional[Exception] = None

    for attempt in range(1, retries + 1):
        label = f" (attempt {attempt}/{retries})" if attempt > 1 else ""
        print(f"\n>>> Running{label}: {' '.join(cmd)}")
        try:
            result = subprocess.run(cmd, capture_output=True, text=True, timeout=timeout_s)
            try:
                with open(CHIP_TOOL_COMMANDS_LOG_PATH, "a", encoding="utf-8") as cmd_log:
                    cmd_log.write(f"\n=== [{time.time():.3f}] CMD{label}: {' '.join(cmd)} (exit={result.returncode}) ===\n")
                    if result.stdout:
                        cmd_log.write(result.stdout)
                    if result.stderr:
                        cmd_log.write(result.stderr)
            except OSError:
                pass
            if result.returncode == 0:
                for line in result.stdout.splitlines():
                    if any(
                        k in line
                        for k in (
                            "Commissioning complete",
                            "Successfully finished commissioning",
                            "Received Command Response",
                            "Data = ",
                            "OnOff:",
                            "PartsList:",
                            "DeviceTypeList:",
                            "ProductName:",
                            "VendorName:",
                            "groupId:",
                            "endpoints:",
                        )
                    ):
                        print(f"  [chip-tool] {line}")
                return result.stdout

            print(f"  [chip-tool] exited with {result.returncode}")
            if attempt == retries:
                print(result.stdout[-4000:])
                print(result.stderr[-4000:], file=sys.stderr)
            last_err = RuntimeError(f"chip-tool command failed (exit {result.returncode}): {' '.join(cmd)}")
        except subprocess.TimeoutExpired as exc:
            print(f"  [chip-tool] timed out after {timeout_s}s")
            last_err = exc

        if attempt < retries:
            time.sleep(2.0)

    assert last_err is not None
    raise last_err


def parse_onoff_state(stdout: str) -> Optional[bool]:
    """Extract OnOff boolean value from chip-tool `onoff read on-off` output."""
    for line in stdout.splitlines():
        m = re.search(r"\bOnOff:\s*(TRUE|FALSE)\b", line, re.IGNORECASE)
        if m:
            return m.group(1).upper() == "TRUE"
    return None


class InteractiveChipTool:
    """Manages a persistent `chip-tool interactive start` process for multi-node subscriptions and group commands."""

    ANSI_ESCAPE_RE = re.compile(r"\x1b\[[0-9;]*[A-Za-z]|\r")

    def __init__(self, chip_tool_bin: str, log_path: str = INTERACTIVE_LOG_PATH) -> None:
        os.makedirs("/tmp/chip_tool_kvs", exist_ok=True)
        self.chip_tool_bin = chip_tool_bin
        self.log_path = log_path
        self.log_file = open(self.log_path, "w", encoding="utf-8", buffering=1)
        self.lines: List[str] = []
        self.cv = threading.Condition()
        self.proc = subprocess.Popen(
            [
                "stdbuf",
                "-oL",
                "-eL",
                chip_tool_bin,
                "interactive",
                "start",
                "--storage-directory",
                "/tmp/chip_tool_kvs",
            ],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            bufsize=1,
        )
        self.reader_thread = threading.Thread(target=self._reader_loop, daemon=True)
        self.reader_thread.start()

    def _reader_loop(self) -> None:
        assert self.proc.stdout is not None
        for raw_line in self.proc.stdout:
            clean_line = self.ANSI_ESCAPE_RE.sub("", raw_line).rstrip("\n")
            if not clean_line.strip():
                continue
            self.log_file.write(clean_line + "\n")
            with self.cv:
                self.lines.append(clean_line)
                self.cv.notify_all()

    def wait_for_ready(self, timeout_s: float = 15.0) -> None:
        deadline = time.time() + timeout_s
        with self.cv:
            while time.time() < deadline:
                if any("CHIP task running" in l or "TransportMgr initialized" in l for l in self.lines):
                    return
                self.cv.wait(timeout=0.2)
        raise RuntimeError("Timed out waiting for chip-tool interactive start to initialize")

    def mark(self) -> int:
        with self.cv:
            return len(self.lines)

    def send_command(self, cmd: str) -> int:
        assert self.proc.stdin is not None
        idx = self.mark()
        print(f"\n>>> [interactive] {cmd}")
        self.proc.stdin.write(cmd + "\n")
        self.proc.stdin.flush()
        return idx

    def run_and_wait(
        self,
        cmd: str,
        success_regex: str,
        timeout_s: float = 60.0,
        retries: int = 3,
    ) -> List[str]:
        pattern = re.compile(success_regex, re.IGNORECASE)
        last_err: Optional[Exception] = None
        for attempt in range(1, retries + 1):
            if attempt > 1:
                print(f"  [interactive] Retrying ({attempt}/{retries}): {cmd}")
                time.sleep(1.5)
            start_idx = self.send_command(cmd)
            deadline = time.time() + timeout_s
            matched = False
            failed_line: Optional[str] = None
            with self.cv:
                while time.time() < deadline:
                    for line in self.lines[start_idx:]:
                        if pattern.search(line):
                            matched = True
                            break
                        if "Run command failure:" in line:
                            failed_line = line
                            break
                    if matched or failed_line is not None:
                        break
                    self.cv.wait(timeout=0.2)

            if matched:
                # Allow RunCommandCleanup on the Matter queue to finish before sending the next command
                time.sleep(0.25)
                with self.cv:
                    return list(self.lines[start_idx:])

            if failed_line is not None:
                print(f"  [interactive] Command failed: {failed_line}")
                last_err = RuntimeError(f"Interactive command failed ({failed_line}): {cmd}")
                time.sleep(0.5)
            else:
                print(f"  [interactive] Timed out after {timeout_s}s waiting for '{success_regex}'")
                last_err = TimeoutError(f"Timed out waiting for '{success_regex}' in command: {cmd}")

        assert last_err is not None
        raise last_err

    def close(self) -> None:
        try:
            if self.proc.poll() is None and self.proc.stdin is not None:
                self.proc.stdin.write("quit\n")
                self.proc.stdin.flush()
                self.proc.wait(timeout=10)
        except Exception:
            if self.proc.poll() is None:
                self.proc.kill()
        finally:
            self.log_file.close()


def collect_subscription_reports_for_window(
    ict: InteractiveChipTool,
    start_idx: int,
    expected_nodes: List[int],
    expected_onoff: bool,
    wait_seconds: float,
) -> Dict[int, Tuple[float, bool]]:
    """Wait `wait_seconds` while tracking incoming IM:ReportData OnOff attribute updates from subscribed nodes.

    Returns a dict mapping node_id -> (controller_epoch_sec, onoff_bool) for the first matching OnOff report.
    """
    expected_str = "TRUE" if expected_onoff else "FALSE"
    t_start = time.time()
    deadline = t_start + wait_seconds
    cursor = start_idx
    current_rx_node: Optional[int] = None
    current_rx_ts: Optional[float] = None
    in_onoff_attr = False
    reported: Dict[int, Tuple[float, bool]] = {}

    rx_re = re.compile(
        r"\[(?P<ts>\d+\.\d+)\].*Msg RX from \d+:0*(?P<node_hex>[0-9a-fA-F]+)\b.*Type 0001:05 \(IM:ReportData\)"
    )
    attr_re = re.compile(
        r"\[(?P<ts>\d+\.\d+)\].*Endpoint:\s*1\s+Cluster:\s*0x0000_0006\s+Attribute\s+0x0000_0000\b"
    )
    val_re = re.compile(r"\[(?P<ts>\d+\.\d+)\].*\bOnOff:\s*(?P<val>TRUE|FALSE)\b", re.IGNORECASE)

    while True:
        remaining = deadline - time.time()
        if remaining <= 0:
            break
        with ict.cv:
            if cursor >= len(ict.lines):
                ict.cv.wait(timeout=min(0.25, max(0.05, remaining)))
            new_lines = ict.lines[cursor:]
            cursor = len(ict.lines)

        for line in new_lines:
            m_rx = rx_re.search(line)
            if m_rx:
                current_rx_ts = float(m_rx.group("ts"))
                current_rx_node = int(m_rx.group("node_hex"), 16)
                in_onoff_attr = False
                continue

            m_attr = attr_re.search(line)
            if m_attr:
                in_onoff_attr = True
                continue

            if in_onoff_attr:
                m_val = val_re.search(line)
                if m_val and current_rx_node is not None:
                    val_bool = m_val.group("val").upper() == "TRUE"
                    ts = current_rx_ts if current_rx_ts is not None else float(m_val.group("ts"))
                    in_onoff_attr = False
                    if current_rx_node not in reported and val_bool == expected_onoff:
                        reported[current_rx_node] = (ts, val_bool)
                        print(
                            f"  [report-rx] Node {current_rx_node} ({len(reported)}/{len(expected_nodes)}) "
                            f"ReportData OnOff={expected_str} at t={ts:.3f}"
                        )
                        if len(reported) >= len(expected_nodes):
                            # All nodes reported; allow 0.45s for final IM:StatusResponse + StandaloneAck
                            # while keeping at least 1.85s separation between consecutive commands.
                            deadline = min(deadline, max(t_start + 1.85, time.time() + 0.45))

    return reported


def verify_single_node(chip_tool_bin: str, node: MatterNodeSpec) -> int:
    """Run legacy single-node commissioning and cluster verification."""
    node_id = str(node.node_id)
    passcode = str(node.passcode)
    discriminator = str(node.discriminator)
    print("\n=== Running End-to-End Verification with chip-tool over LAN -> OTBR -> OTNS Thread ===")
    run_chip_tool(chip_tool_bin, ["pairing", "onnetwork-long", node_id, passcode, discriminator])
    run_chip_tool(chip_tool_bin, ["basicinformation", "read", "vendor-name", node_id, "0"])
    run_chip_tool(chip_tool_bin, ["basicinformation", "read", "product-name", node_id, "0"])
    run_chip_tool(chip_tool_bin, ["descriptor", "read", "parts-list", node_id, "0"])
    run_chip_tool(chip_tool_bin, ["descriptor", "read", "device-type-list", node_id, "1"])
    first_dev = node.devices[0] if node.devices else ""
    if first_dev.startswith("on-off-light") or first_dev.startswith("dimmable-light"):
        run_chip_tool(chip_tool_bin, ["onoff", "on", node_id, "1"])
        run_chip_tool(chip_tool_bin, ["onoff", "read", "on-off", node_id, "1"])
        run_chip_tool(chip_tool_bin, ["onoff", "off", node_id, "1"])
        run_chip_tool(chip_tool_bin, ["onoff", "read", "on-off", node_id, "1"])
    print("\n=== SUCCESS: End-to-end commissioning and cluster control verified! ===")
    return 0


def verify_multi_node_and_groups(
    ns: OTNS,
    otbr_id: int,
    chip_tool_bin: str,
    env_cfg: EnvironmentConfig,
    state_reused: bool = False,
    subscribe_wildcard: bool = False,
    group_wait_sec: float = 60.0,
    group_cycles: int = 1,
    delay_report_min_ms: Optional[int] = None,
    delay_report_jitter_ms: Optional[int] = None,
) -> int:
    """Commission all configured nodes (unless state_reused), join Groupcast groups, and verify group On/Off."""
    nodes = env_cfg.matter_nodes
    groups_by_id = {g.group_id: g for g in env_cfg.groups}
    total = len(nodes)

    if state_reused:
        print(f"\n=== [Step 1/4] Skipping Commissioning (reused persisted fabric & credentials for {total} nodes) ===")
        print(f"=== [Step 2/4] Skipping Groupcast JoinGroup (reused persisted Groupcast 0x0065 state for {total} nodes) ===")
    else:
        print(f"\n=== [Step 1/4] Commissioning {total} Matter Nodes via OTBR ===")
        for idx, node in enumerate(nodes, start=1):
            print(
                f"\n--- Commissioning node {idx}/{total}: "
                f"node_id={node.node_id}, discriminator={node.discriminator}, otns_id={node.otns_id} ---"
            )
            run_chip_tool(
                chip_tool_bin,
                [
                    "pairing",
                    "onnetwork-long",
                    str(node.node_id),
                    str(node.passcode),
                    str(node.discriminator),
                ],
                timeout_s=90,
                retries=3,
            )

        if env_cfg.groups:
            print(f"\n=== [Step 2/4] Configuring Groupcast (0x0065) on Controller and {total} Nodes ===")
            for group in env_cfg.groups:
                print(
                    f"\n--- Configuring Controller groupsettings for {group.name} "
                    f"(group_id={group.group_id}, keyset_id={group.keyset_id}, policy={group.mcast_addr_policy}) ---"
                )
                run_chip_tool(
                    chip_tool_bin,
                    [
                        "groupsettings",
                        "add-group",
                        group.name,
                        group.group_id,
                        "--addressPolicy",
                        str(group.mcast_addr_policy),
                    ],
                    retries=1,
                )
                if group.group_id_int not in (0x0101, 0x0102, 0x0103):
                    run_chip_tool(
                        chip_tool_bin,
                        [
                            "groupsettings",
                            "add-keysets",
                            group.keyset_id,
                            "0",
                            "1000000",
                            f"hex:{group.key}",
                        ],
                        retries=1,
                    )
                    run_chip_tool(
                        chip_tool_bin,
                        [
                            "groupsettings",
                            "bind-keyset",
                            group.group_id,
                            group.keyset_id,
                        ],
                        retries=1,
                    )
                run_chip_tool(
                    chip_tool_bin,
                    [
                        "groupsettings",
                        "show-groups",
                    ],
                    retries=1,
                )

            for idx, node in enumerate(nodes, start=1):
                for gid in node.groups:
                    group = groups_by_id[gid]
                    endpoints_json = json.dumps(group.endpoints)
                    print(
                        f"\n--- Joining node {idx}/{total} (node_id={node.node_id}) to Groupcast "
                        f"group {group.group_id} on endpoints {endpoints_json} ---"
                    )
                    run_chip_tool(
                        chip_tool_bin,
                        [
                            "groupcast",
                            "join-group",
                            group.group_id,
                            endpoints_json,
                            group.keyset_id,
                            str(node.node_id),
                            "0",
                            "--Key",
                            f"hex:{group.key}",
                            "--UseAuxiliaryACL",
                            "1" if group.use_auxiliary_acl else "0",
                            "--McastAddrPolicy",
                            str(group.mcast_addr_policy),
                        ],
                        timeout_s=60,
                        retries=3,
                    )
        save_commissioned_state(env_cfg)

    on_results: Dict[int, Optional[bool]] = {}
    off_results: Dict[int, Optional[bool]] = {}

    if subscribe_wildcard:
        print(f"\n=== [Wildcard Subscription Mode] Starting single persistent chip-tool interactive controller ===")
        ict = InteractiveChipTool(chip_tool_bin)
        try:
            ict.wait_for_ready(timeout_s=15.0)
            for group in env_cfg.groups:
                ict.run_and_wait(
                    f"groupsettings add-group {group.name} {group.group_id} --addressPolicy {group.mcast_addr_policy}",
                    r"Command: groupsettings add-group",
                    timeout_s=15.0,
                )
                if group.group_id_int not in (0x0101, 0x0102, 0x0103):
                    ict.run_and_wait(
                        f"groupsettings add-keysets {group.keyset_id} 0 1000000 hex:{group.key}",
                        r"Command: groupsettings add-keysets",
                        timeout_s=15.0,
                    )
                    ict.run_and_wait(
                        f"groupsettings bind-keyset {group.group_id} {group.keyset_id}",
                        r"Command: groupsettings bind-keyset",
                        timeout_s=15.0,
                    )
                ict.run_and_wait(
                    "groupsettings show-groups",
                    r"Available Groups",
                    timeout_s=15.0,
                )

            sub_max_interval = max(300, int(group_cycles * group_wait_sec * 3) + 600)
            print(
                f"\n=== Establishing Wildcard Subscriptions (maxInterval={sub_max_interval}s) "
                f"from Single Controller to {total} Matter Nodes ==="
            )
            for idx, node in enumerate(nodes, start=1):
                print(
                    f"\n--- Subscribing wildcard (0xFFFF/0xFFFFFFFF/0xFFFFFFFF) to node {idx}/{total} "
                    f"(node_id={node.node_id}, otns_id={node.otns_id}) ---"
                )
                sub_lines = ict.run_and_wait(
                    f"any subscribe-by-id 0xFFFFFFFF 0xFFFFFFFF 0 {sub_max_interval} {node.node_id} 0xFFFF --keepSubscriptions false",
                    rf"Subscription established.*Peer = \d+:0*{node.node_id:x}\b",
                    timeout_s=90.0,
                    retries=3,
                )
                for line in sub_lines:
                    if "Subscription established" in line:
                        print(f"  [sub-ok] {line}")

            for group in env_cfg.groups:
                mcast_dest = group.multicast_node_id_hex
                ep_str = str(group.endpoints[0])
                member_nodes = [n for n in nodes if group.group_id in n.groups]
                expected_node_ids = [n.node_id for n in member_nodes]

                omr_prefix = ns.node_cmd(otbr_id, "br omrprefix")
                configure_border_router_routes(omr_prefix)

                delay_args_str = ""
                if delay_report_min_ms is not None:
                    delay_args_str += f" --delay-report-min-ms {delay_report_min_ms}"
                if delay_report_jitter_ms is not None:
                    delay_args_str += f" --delay-report-jitter-ms {delay_report_jitter_ms}"

                for cycle_idx in range(1, max(1, group_cycles) + 1):
                    if cycle_idx > 1 and (cycle_idx - 1) % 10 == 0:
                        configure_border_router_routes(omr_prefix)

                    cycle_label = f" (Cycle {cycle_idx}/{group_cycles})" if group_cycles > 1 else ""
                    print(
                        f"\n=== [Step 3/4{cycle_label}] Issuing Groupcast ON Command to {mcast_dest} "
                        f"(Endpoint {ep_str}{delay_args_str}) and waiting {group_wait_sec:.1f}s for ReportData updates ==="
                    )
                    on_start_idx = ict.mark()
                    ict.run_and_wait(
                        f"onoff on {mcast_dest} {ep_str}{delay_args_str}",
                        r"Successfully send Multicast message",
                        timeout_s=15.0,
                        retries=1,
                    )
                    on_reports = collect_subscription_reports_for_window(
                        ict,
                        start_idx=on_start_idx,
                        expected_nodes=expected_node_ids,
                        expected_onoff=True,
                        wait_seconds=group_wait_sec,
                    )
                    for n_id in expected_node_ids:
                        if n_id in on_reports:
                            on_results[n_id] = on_reports[n_id][1]

                    print(
                        f"\n=== [Step 4/4{cycle_label}] Issuing Groupcast OFF Command to {mcast_dest} "
                        f"(Endpoint {ep_str}{delay_args_str}) and waiting {group_wait_sec:.1f}s for ReportData updates ==="
                    )
                    off_start_idx = ict.mark()
                    ict.run_and_wait(
                        f"onoff off {mcast_dest} {ep_str}{delay_args_str}",
                        r"Successfully send Multicast message",
                        timeout_s=15.0,
                        retries=1,
                    )
                    off_reports = collect_subscription_reports_for_window(
                        ict,
                        start_idx=off_start_idx,
                        expected_nodes=expected_node_ids,
                        expected_onoff=False,
                        wait_seconds=group_wait_sec,
                    )
                    for n_id in expected_node_ids:
                        if n_id in off_reports:
                            off_results[n_id] = off_reports[n_id][1]
        finally:
            ict.close()
            save_commissioned_state(env_cfg)
    else:
        for group in env_cfg.groups:
            mcast_dest = group.multicast_node_id_hex
            ep_str = str(group.endpoints[0])
            member_nodes = [n for n in nodes if group.group_id in n.groups]

            # Ensure Border Router multicast routes are still present before sending groupcast
            omr_prefix = ns.node_cmd(otbr_id, "br omrprefix")
            configure_border_router_routes(omr_prefix)

            print(
                f"\n=== [Step 3/4] Issuing Groupcast ON Command to {mcast_dest} "
                f"(Endpoint {ep_str}) for {len(member_nodes)} nodes ==="
            )
            run_chip_tool(chip_tool_bin, ["onoff", "on", mcast_dest, ep_str], retries=1)
            time.sleep(3.0)

            for idx, node in enumerate(member_nodes, start=1):
                stdout = run_chip_tool(
                    chip_tool_bin,
                    ["onoff", "read", "on-off", str(node.node_id), ep_str],
                    timeout_s=60,
                    retries=3,
                )
                state = parse_onoff_state(stdout)
                on_results[node.node_id] = state
                print(f"  [verify-on] Node {node.node_id} ({idx}/{len(member_nodes)}) OnOff = {state}")
                if state is not True:
                    raise RuntimeError(
                        f"Node {node.node_id} expected OnOff=True after group ON, got {state}"
                    )

            print(
                f"\n=== [Step 4/4] Issuing Groupcast OFF Command to {mcast_dest} "
                f"(Endpoint {ep_str}) for {len(member_nodes)} nodes ==="
            )
            run_chip_tool(chip_tool_bin, ["onoff", "off", mcast_dest, ep_str], retries=1)
            time.sleep(3.0)

            for idx, node in enumerate(member_nodes, start=1):
                stdout = run_chip_tool(
                    chip_tool_bin,
                    ["onoff", "read", "on-off", str(node.node_id), ep_str],
                    timeout_s=60,
                    retries=3,
                )
                state = parse_onoff_state(stdout)
                off_results[node.node_id] = state
                print(f"  [verify-off] Node {node.node_id} ({idx}/{len(member_nodes)}) OnOff = {state}")
                if state is not False:
                    raise RuntimeError(
                        f"Node {node.node_id} expected OnOff=False after group OFF, got {state}"
                    )
        save_commissioned_state(env_cfg)

    print("\n=== Verification Summary Table ===")
    print(
        f"{'OTNS ID':>7} | {'Node ID':>7} | {'Disc':>6} | {'Thread Role':>11} | "
        f"{'Groups':>8} | {'Group ON':>8} | {'Group OFF':>9}"
    )
    print("-" * 74)
    for node in nodes:
        assert node.otns_id is not None
        role_lines = ns.node_cmd(node.otns_id, "state")
        role = role_lines[0].strip() if role_lines else "unknown"
        groups_str = ",".join(node.groups) if node.groups else "-"
        on_str = str(on_results.get(node.node_id, "-"))
        off_str = str(off_results.get(node.node_id, "-"))
        print(
            f"{node.otns_id:>7} | {node.node_id:>7} | {node.discriminator:>6} | "
            f"{role:>11} | {groups_str:>8} | {on_str:>8} | {off_str:>9}"
        )

    print(
        f"\n=== SUCCESS: All {total} nodes verified with Groupcast (0x0065) group ON/OFF commands! ==="
    )
    return 0


def sanitize_slug(value: str) -> str:
    """Convert an arbitrary string into a filesystem-safe lowercase slug."""
    slug = re.sub(r"[^a-zA-Z0-9]+", "_", value.strip().lower()).strip("_")
    return slug or "run"


def build_run_id(
    args: argparse.Namespace,
    env_cfg: EnvironmentConfig,
    state_reused: bool,
    started_epoch: float,
) -> str:
    """Construct a descriptive, chronologically sortable subdirectory name for a simulation run.

    Format: YYYYMMDD_HHMMSS_<scenario>_<topology>_<mode>[_<run_tag>]
    Example: 20261014_103852_20_onoff_group_1otbr_4rtr_20matter_reuse_wildcard
    """
    try:
        local_tz = ZoneInfo(os.environ.get("TZ", "America/Los_Angeles"))
        ts_str = datetime.fromtimestamp(started_epoch, tz=local_tz).strftime("%Y%m%d_%H%M%S")
    except Exception:
        ts_str = datetime.fromtimestamp(started_epoch).strftime("%Y%m%d_%H%M%S")
    if args.config:
        cfg_stem = os.path.splitext(os.path.basename(args.config))[0]
        if cfg_stem.startswith("otns_"):
            cfg_stem = cfg_stem[len("otns_") :]
        scenario = sanitize_slug(cfg_stem)
    else:
        dev_name = args.device.split(":")[0]
        scenario = f"single_{sanitize_slug(dev_name)}"

    topology = f"1otbr_{len(env_cfg.routers)}rtr_{len(env_cfg.matter_nodes)}matter"

    state_prefix = "reuse" if state_reused else "fresh"
    if args.subscribe_wildcard:
        mode = f"{state_prefix}_wildcard"
    elif args.verify and env_cfg.groups:
        mode = f"{state_prefix}_groupcast"
    elif args.verify:
        mode = f"{state_prefix}_verify"
    else:
        mode = f"{state_prefix}_interactive"

    parts = [ts_str, scenario, topology, mode]
    if getattr(args, "run_tag", None):
        parts.append(sanitize_slug(args.run_tag))
    return "_".join(parts)


def extract_last_rloc16(node_log_path: str) -> Optional[str]:
    """Scan an OTNS node log for its most recently assigned RLOC16."""
    if not os.path.isfile(node_log_path):
        return None
    rloc: Optional[str] = None
    rloc_re = re.compile(r"RLOC16\s+([0-9a-fA-F]{4})\b")
    try:
        with open(node_log_path, "r", encoding="utf-8", errors="replace") as f:
            for line in f:
                m = rloc_re.search(line)
                if m:
                    rloc = f"0x{m.group(1).lower()}"
    except OSError:
        pass
    return rloc


def archive_simulation_logs(
    args: argparse.Namespace,
    env_cfg: EnvironmentConfig,
    state_reused: bool,
    started_epoch: float,
    completed_epoch: float,
    exit_code: int,
) -> Optional[str]:
    """Copy all OTNS node logs, PCAP, controller logs, daemon logs, and manifest into <log_dir>/<run_id>/."""
    if getattr(args, "no_archive_logs", False) or not args.log_dir:
        return None

    run_id = build_run_id(args, env_cfg, state_reused=state_reused, started_epoch=started_epoch)
    run_dir = os.path.join(args.log_dir, run_id)
    nodes_dir = os.path.join(run_dir, "nodes")
    controller_dir = os.path.join(run_dir, "controller")
    daemons_dir = os.path.join(run_dir, "daemons")
    pcap_dir = os.path.join(run_dir, "pcap")

    for d in (nodes_dir, controller_dir, daemons_dir, pcap_dir):
        os.makedirs(d, exist_ok=True)

    # 1. Save config snapshot
    config_dst = os.path.join(run_dir, "config.json")
    if args.config and os.path.isfile(args.config):
        shutil.copy2(args.config, config_dst)
    else:
        with open(config_dst, "w", encoding="utf-8") as f:
            json.dump(asdict(env_cfg), f, indent=2)

    # 2. Build topology map (OTNS ID -> role / Matter node metadata) and copy node logs + friendly symlinks
    num_routers = len(env_cfg.routers)
    node_manifest: List[Dict[str, Any]] = []

    # Node 1: OTBR
    otbr_raw_name = "0_1.log"
    otbr_src = os.path.join(OT_TMP_DIR, otbr_raw_name)
    otbr_friendly = "otns_01_otbr.log"
    if os.path.isfile(otbr_src):
        shutil.copy2(otbr_src, os.path.join(nodes_dir, otbr_raw_name))
        link_path = os.path.join(nodes_dir, otbr_friendly)
        if not os.path.exists(link_path):
            os.symlink(otbr_raw_name, link_path)
    node_manifest.append(
        {
            "otns_id": 1,
            "role": "otbr",
            "matter_node_id": None,
            "discriminator": None,
            "rloc16": extract_last_rloc16(otbr_src),
            "x": env_cfg.otbr_x,
            "y": env_cfg.otbr_y,
            "radio_range": env_cfg.otbr_radio_range,
            "raw_log": f"nodes/{otbr_raw_name}",
            "named_log": f"nodes/{otbr_friendly}",
        }
    )

    # Nodes 2 .. 1 + num_routers: Thread FTD routers
    for idx, rtr in enumerate(env_cfg.routers, start=1):
        otns_id = 1 + idx
        raw_name = f"0_{otns_id}.log"
        src_path = os.path.join(OT_TMP_DIR, raw_name)
        friendly_name = f"otns_{otns_id:02d}_router_{idx:02d}.log"
        if os.path.isfile(src_path):
            shutil.copy2(src_path, os.path.join(nodes_dir, raw_name))
            link_path = os.path.join(nodes_dir, friendly_name)
            if not os.path.exists(link_path):
                os.symlink(raw_name, link_path)
        node_manifest.append(
            {
                "otns_id": otns_id,
                "role": f"router_{idx}",
                "matter_node_id": None,
                "discriminator": None,
                "rloc16": extract_last_rloc16(src_path),
                "x": int(rtr.get("x", 375)),
                "y": int(rtr.get("y", 180 + (idx - 1) * 120)),
                "radio_range": rtr.get("radio_range", 250),
                "raw_log": f"nodes/{raw_name}",
                "named_log": f"nodes/{friendly_name}",
            }
        )

    # Matter nodes: 2 + num_routers ..
    for idx, mnode in enumerate(env_cfg.matter_nodes, start=1):
        otns_id = mnode.otns_id if mnode.otns_id is not None else (1 + num_routers + idx)
        raw_name = f"0_{otns_id}.log"
        src_path = os.path.join(OT_TMP_DIR, raw_name)
        friendly_name = f"otns_{otns_id:02d}_matter_{mnode.node_id}.log"
        if os.path.isfile(src_path):
            shutil.copy2(src_path, os.path.join(nodes_dir, raw_name))
            link_path = os.path.join(nodes_dir, friendly_name)
            if not os.path.exists(link_path):
                os.symlink(raw_name, link_path)
        node_manifest.append(
            {
                "otns_id": otns_id,
                "role": "matter_router_eligible" if mnode.router_eligible else "matter_med",
                "matter_node_id": mnode.node_id,
                "discriminator": mnode.discriminator,
                "devices": mnode.devices,
                "groups": mnode.groups,
                "rloc16": extract_last_rloc16(src_path),
                "x": mnode.x,
                "y": mnode.y,
                "radio_range": mnode.radio_range,
                "raw_log": f"nodes/{raw_name}",
                "named_log": f"nodes/{friendly_name}",
            }
        )

    # Copy any additional 0_*.log files if interactive nodes were added dynamically
    known_raw = {os.path.basename(item["raw_log"]) for item in node_manifest}
    for extra_log in sorted(glob.glob(os.path.join(OT_TMP_DIR, "0_*.log"))):
        bname = os.path.basename(extra_log)
        if bname not in known_raw:
            shutil.copy2(extra_log, os.path.join(nodes_dir, bname))

    # 3. Copy controller logs
    controller_files: List[str] = []
    if os.path.isfile(INTERACTIVE_LOG_PATH):
        shutil.copy2(INTERACTIVE_LOG_PATH, os.path.join(controller_dir, "chip_tool_interactive.log"))
        controller_files.append("controller/chip_tool_interactive.log")
    if os.path.isfile(CHIP_TOOL_COMMANDS_LOG_PATH):
        shutil.copy2(CHIP_TOOL_COMMANDS_LOG_PATH, os.path.join(controller_dir, "chip_tool_commands.log"))
        controller_files.append("controller/chip_tool_commands.log")

    # 4. Copy daemon logs (OTNS dispatcher, otbr-agent stderr, mdnsd)
    daemon_files: List[str] = []
    for src_daemon, dst_name in [
        (os.path.join(OT_TMP_DIR, "0.log"), "otns_dispatcher.log"),
        ("/tmp/otns.log", "otns_cli.log"),
        ("/tmp/otbr_stderr.log", "otbr_agent_stderr.log"),
        ("/tmp/mdnsd.log", "mdnsd.log"),
    ]:
        if os.path.isfile(src_daemon):
            shutil.copy2(src_daemon, os.path.join(daemons_dir, dst_name))
            daemon_files.append(f"daemons/{dst_name}")

    # 5. Copy 802.15.4 PCAP capture, OTNS replay log, and per-node radio stats CSV
    pcap_files: List[str] = []
    for search_dir in (OT_VERSIONS_DIR, OT_TMP_DIR):
        for artifact_name in ("current.pcap", "otns_0.replay"):
            src_artifact = os.path.join(search_dir, artifact_name)
            dst_rel = f"pcap/{artifact_name}"
            if os.path.isfile(src_artifact) and dst_rel not in pcap_files:
                shutil.copy2(src_artifact, os.path.join(pcap_dir, artifact_name))
                pcap_files.append(dst_rel)

    stats_csv_rel: Optional[str] = None
    src_stats_csv = os.path.join(OT_TMP_DIR, "0_stats.csv")
    if os.path.isfile(src_stats_csv):
        shutil.copy2(src_stats_csv, os.path.join(run_dir, "stats.csv"))
        stats_csv_rel = "stats.csv"

    # 6. Write manifest.json
    try:
        local_tz = ZoneInfo(os.environ.get("TZ", "America/Los_Angeles"))
        started_local = datetime.fromtimestamp(started_epoch, tz=local_tz).isoformat()
        completed_local = datetime.fromtimestamp(completed_epoch, tz=local_tz).isoformat()
    except Exception:
        started_local = datetime.fromtimestamp(started_epoch, tz=timezone.utc).isoformat()
        completed_local = datetime.fromtimestamp(completed_epoch, tz=timezone.utc).isoformat()

    manifest = {
        "run_id": run_id,
        "started_at": started_local,
        "completed_at": completed_local,
        "started_at_utc": datetime.fromtimestamp(started_epoch, tz=timezone.utc).isoformat(),
        "completed_at_utc": datetime.fromtimestamp(completed_epoch, tz=timezone.utc).isoformat(),
        "duration_sec": round(completed_epoch - started_epoch, 3),
        "exit_code": exit_code,
        "cli_argv": sys.argv,
        "config_path": args.config,
        "state_reused": state_reused,
        "subscribe_wildcard": args.subscribe_wildcard,
        "group_wait_sec": args.group_wait_sec if args.subscribe_wildcard else None,
        "group_cycles": args.group_cycles if args.subscribe_wildcard else 1,
        "delay_report_min_ms": args.delay_report_min_ms,
        "delay_report_jitter_ms": args.delay_report_jitter_ms,
        "counts": {
            "otbr": 1,
            "routers": len(env_cfg.routers),
            "matter_nodes": len(env_cfg.matter_nodes),
            "groups": len(env_cfg.groups),
        },
        "files": {
            "config": "config.json",
            "stats_csv": stats_csv_rel,
            "timing_breakdown": "timing_breakdown.txt",
            "controller": controller_files,
            "daemons": daemon_files,
            "pcap": pcap_files,
        },
        "nodes": node_manifest,
    }
    manifest_path = os.path.join(run_dir, "manifest.json")
    with open(manifest_path, "w", encoding="utf-8") as f:
        json.dump(manifest, f, indent=2)

    # 7. Generate timing_breakdown.txt if group multicast logs are present
    try:
        from parse_otns_group_timing import analyze_run_dir

        analyze_run_dir(run_dir, write_report=True)
    except Exception:
        pass

    # 8. Update `latest` relative symlink and append to `runs_index.jsonl`
    latest_link = os.path.join(args.log_dir, "latest")
    try:
        if os.path.islink(latest_link) or os.path.exists(latest_link):
            os.unlink(latest_link)
        os.symlink(run_id, latest_link)
    except OSError:
        pass

    index_entry = {
        "run_id": run_id,
        "started_at": manifest["started_at"],
        "completed_at": manifest["completed_at"],
        "duration_sec": manifest["duration_sec"],
        "exit_code": exit_code,
        "state_reused": state_reused,
        "subscribe_wildcard": args.subscribe_wildcard,
        "matter_nodes": len(env_cfg.matter_nodes),
        "path": run_dir,
    }
    try:
        with open(os.path.join(args.log_dir, "runs_index.jsonl"), "a", encoding="utf-8") as idx_f:
            idx_f.write(json.dumps(index_entry) + "\n")
    except OSError:
        pass

    # 9. Match ownership of archived files to /workspace so the host user owns them cleanly
    try:
        ws_stat = os.stat("/workspace")
        ws_uid, ws_gid = ws_stat.st_uid, ws_stat.st_gid
        for target in (args.log_dir, latest_link, os.path.join(args.log_dir, "runs_index.jsonl")):
            if os.path.lexists(target):
                os.lchown(target, ws_uid, ws_gid)
        for root, dirs, files in os.walk(run_dir):
            os.lchown(root, ws_uid, ws_gid)
            for dname in dirs:
                os.lchown(os.path.join(root, dname), ws_uid, ws_gid)
            for fname in files:
                os.lchown(os.path.join(root, fname), ws_uid, ws_gid)
    except OSError:
        pass

    print(f"\n=== Archived simulation logs to: {run_dir} (symlink: {latest_link}) ===")
    return run_dir


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Run OTNS + simulated OTBR + Matter all-devices-app + chip-tool environment"
    )
    parser.add_argument(
        "--config",
        default=None,
        help="Path to JSON configuration file specifying OTBR, groups, and Matter nodes",
    )
    parser.add_argument(
        "--device",
        default="on-off-light:1",
        help="Device specification passed to all-devices-app --device when --config is not used (default: on-off-light:1)",
    )
    parser.add_argument(
        "--num-routers",
        type=int,
        default=1,
        help="Additional Thread FTD router nodes to spawn when --config is not used (default: 1)",
    )
    parser.add_argument(
        "--node-id",
        type=int,
        default=1234,
        help="Matter Node ID to assign when --config is not used (default: 1234)",
    )
    parser.add_argument(
        "--discriminator",
        type=int,
        default=3840,
        help="Matter setup discriminator when --config is not used (default: 3840)",
    )
    parser.add_argument(
        "--passcode",
        type=int,
        default=20202021,
        help="Matter setup passcode when --config is not used (default: 20202021)",
    )
    parser.add_argument(
        "--all-devices-bin",
        default=DEFAULT_ALL_DEVICES_BIN,
        help="Path to all-devices-app binary built with OTNS support",
    )
    parser.add_argument(
        "--chip-tool-bin",
        default=DEFAULT_CHIP_TOOL_BIN,
        help="Path to chip-tool binary",
    )
    parser.add_argument(
        "--reuse-state",
        action="store_true",
        help="Reuse commissioned KVS state and Groupcast memberships from previous simulation if available",
    )
    parser.add_argument(
        "--subscribe-wildcard",
        action="store_true",
        help="Establish wildcard subscriptions from a single persistent chip-tool process to all nodes before group ON/OFF",
    )
    parser.add_argument(
        "--group-wait-sec",
        type=float,
        default=60.0,
        help="Seconds to wait after Group ON and Group OFF when --subscribe-wildcard is used (default: 60.0)",
    )
    parser.add_argument(
        "--group-cycles",
        type=int,
        default=1,
        help="Number of Group ON + OFF cycles to execute when --subscribe-wildcard is used (default: 1)",
    )
    parser.add_argument(
        "--delay-report-min-ms",
        type=int,
        default=None,
        help="Optional DelayReportData.delayMinMs (ms) passed to group ON/OFF InvokeRequestMessage",
    )
    parser.add_argument(
        "--delay-report-jitter-ms",
        type=int,
        default=None,
        help="Optional DelayReportData.delayJitterWindowMs (ms) passed to group ON/OFF InvokeRequestMessage",
    )
    parser.add_argument(
        "--log-dir",
        default=DEFAULT_SIM_LOGS_DIR,
        help=f"Base directory to archive simulation logs per run (default: {DEFAULT_SIM_LOGS_DIR})",
    )
    parser.add_argument(
        "--run-tag",
        default=None,
        help="Optional descriptive tag appended to the run log subdirectory name",
    )
    parser.add_argument(
        "--no-archive-logs",
        action="store_true",
        help="Disable automatic archiving of simulation logs at the end of the run",
    )
    parser.add_argument(
        "--archive-existing",
        action="store_true",
        help="Archive logs from the most recent completed run in OT_TMP_DIR / /tmp without starting a new simulation",
    )
    parser.add_argument(
        "--verify",
        action="store_true",
        help="Run automated end-to-end commissioning and cluster/group verification with chip-tool, then exit",
    )
    args = parser.parse_args()

    env_cfg = load_environment_config(args)

    if args.archive_existing:
        otbr_log = os.path.join(OT_TMP_DIR, "0_1.log")
        ct_log = INTERACTIVE_LOG_PATH if os.path.isfile(INTERACTIVE_LOG_PATH) else otbr_log
        completed_ts = os.path.getmtime(ct_log) if os.path.isfile(ct_log) else time.time()
        started_ts = completed_ts
        if os.path.isfile(otbr_log):
            # Estimate run start time from OTBR log last SimTimeUs
            try:
                with open(otbr_log, "r", encoding="utf-8", errors="replace") as f:
                    lines = f.readlines()
                for line in reversed(lines):
                    m = re.match(r"^\s*(\d+)\s+", line)
                    if m:
                        sim_duration_s = int(m.group(1)) / 1_000_000.0
                        started_ts = max(0.0, completed_ts - sim_duration_s)
                        break
            except OSError:
                pass
        archive_simulation_logs(
            args,
            env_cfg,
            state_reused=args.reuse_state,
            started_epoch=started_ts,
            completed_epoch=completed_ts,
            exit_code=0,
        )
        return 0

    started_epoch = time.time()
    exit_code = 1
    backbone_if = detect_backbone_interface()
    print(f"Using backbone LAN interface: {backbone_if}")
    print(f"Using all-devices-app binary: {args.all_devices_bin}")
    print(f"Using chip-tool binary:       {args.chip_tool_bin}")
    print(f"Configured Matter nodes:      {len(env_cfg.matter_nodes)}")
    print(f"Configured Groupcast groups:  {len(env_cfg.groups)}")

    os.chdir(OT_VERSIONS_DIR)
    state_reused = prepare_system(
        backbone_if,
        args.all_devices_bin,
        env_cfg,
        reuse_state=args.reuse_state,
    )

    ns = OTNS(
        otns_args=[
            "-realtime",
            "-logfile",
            "info",
            "-listen",
            "0.0.0.0:9000",
            "-web=true",
            "-otbr-backbone-if",
            backbone_if,
        ]
    )

    try:
        ns.radiomodel = "MIDisc"
        print("\n[1/4] Spawning simulated Thread Border Router (OTBR)...")
        otbr_id = ns.add(
            "otbr",
            x=env_cfg.otbr_x,
            y=env_cfg.otbr_y,
            radio_range=env_cfg.otbr_radio_range,
        )
        ns.autogo = True
        if not wait_for_otbr_ready(ns, otbr_id=otbr_id, timeout_s=40):
            print("ERROR: Timed out waiting for OTBR to become ready.", file=sys.stderr)
            return 1

        router_ids: List[int] = []
        for i, rtr in enumerate(env_cfg.routers):
            print(f"[2/4] Spawning Thread router {i + 1}/{len(env_cfg.routers)}...")
            rtr_id = ns.add(
                "router",
                x=int(rtr.get("x", 375)),
                y=int(rtr.get("y", 180 + i * 120)),
                radio_range=rtr.get("radio_range", 250),
            )
            ns.node_cmd(rtr_id, "routerselectionjitter 1")
            router_ids.append(rtr_id)
            time.sleep(1.0)

        if router_ids and not wait_for_routers_ready(ns, router_ids, timeout_s=35):
            print("ERROR: Timed out waiting for Thread routers to attach.", file=sys.stderr)
            return 1

        total_nodes = len(env_cfg.matter_nodes)
        batch_size = max(1, env_cfg.batch_size)
        print(
            f"[3/4] Spawning {total_nodes} simulated Matter all-devices-app node(s) "
            f"in batches of {batch_size}..."
        )
        spawned_nodes: List[MatterNodeSpec] = []
        for batch_start in range(0, total_nodes, batch_size):
            batch = env_cfg.matter_nodes[batch_start : batch_start + batch_size]
            for node in batch:
                assert node.wrapper_path is not None
                node.otns_id = ns.add(
                    "matter",
                    x=node.x,
                    y=node.y,
                    radio_range=node.radio_range,
                    executable=node.wrapper_path,
                )
                if not node.router_eligible:
                    ns.node_cmd(node.otns_id, "routereligible disable")
                spawned_nodes.append(node)
                print(
                    f"  Spawned Matter node {len(spawned_nodes)}/{total_nodes}: "
                    f"otns_id={node.otns_id}, node_id={node.node_id}, "
                    f"discriminator={node.discriminator}, pos=({node.x},{node.y}), "
                    f"router_eligible={node.router_eligible}"
                )
                time.sleep(1.0)

            print(
                f"[4/4] Waiting for mesh attachment and SRP registration "
                f"({len(spawned_nodes)}/{total_nodes} nodes)..."
            )
            if not wait_for_mesh_and_srp(
                ns,
                otbr_id=otbr_id,
                matter_nodes=spawned_nodes,
                timeout_s=75,
            ):
                print("ERROR: Timed out waiting for Thread mesh / SRP registration.", file=sys.stderr)
                return 1

        # Give mDNSResponder / border routing a moment to finish advertising all records on backbone_if
        time.sleep(5.0)
        omr_prefix = ns.node_cmd(otbr_id, "br omrprefix")
        configure_border_router_routes(omr_prefix)

        if args.verify:
            if len(env_cfg.matter_nodes) == 1 and not env_cfg.groups and not args.subscribe_wildcard:
                exit_code = verify_single_node(args.chip_tool_bin, env_cfg.matter_nodes[0])
                return exit_code
            exit_code = verify_multi_node_and_groups(
                ns,
                otbr_id,
                args.chip_tool_bin,
                env_cfg,
                state_reused=state_reused,
                subscribe_wildcard=args.subscribe_wildcard,
                group_wait_sec=args.group_wait_sec,
                group_cycles=args.group_cycles,
                delay_report_min_ms=args.delay_report_min_ms,
                delay_report_jitter_ms=args.delay_report_jitter_ms,
            )
            return exit_code

        print("\n=== Environment Ready ===")
        print("OTNS Web UI: http://localhost:8997/visualize?addr=localhost:8998")
        for node in env_cfg.matter_nodes:
            print(
                f"  Node {node.node_id} (otns_id={node.otns_id}): "
                f"chip-tool pairing onnetwork-long {node.node_id} {node.passcode} {node.discriminator}"
            )
        ns.interactive_cli()
        exit_code = 0
        return 0
    finally:
        ns.close()
        archive_simulation_logs(
            args,
            env_cfg,
            state_reused=state_reused,
            started_epoch=started_epoch,
            completed_epoch=time.time(),
            exit_code=exit_code,
        )


if __name__ == "__main__":
    sys.exit(main())
