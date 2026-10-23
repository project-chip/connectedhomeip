# OpenThread Network Simulator (OTNS) for Matter Linux Examples

This directory contains tools, declarative home topologies, and analysis scripts for running simulated Matter over Thread home networks using the [OpenThread Network Simulator (OTNS)](https://github.com/openthread/ot-ns) (`third_party/ot-ns/repo`).

## 1. Building Matter Examples with OTNS Support

Linux example targets that use the OpenThread Inet endpoint (`-openthread-endpoint`) can be built against the OTNS virtual-time RF simulator platform (`third_party/openthread/platforms/otns`) by appending `-otns` to the target name:

```bash
# Build all-devices-app with OpenThread Endpoint + OTNS support
./scripts/run_in_build_env.sh \
  "./scripts/build/build_examples.py \
    --target linux-x64-all-devices-no-wifi-no-ble-openthread-endpoint-otns \
    build"

# Build all-clusters-app with OpenThread Endpoint + OTNS support
./scripts/run_in_build_env.sh \
  "./scripts/build/build_examples.py \
    --target linux-x64-all-clusters-no-wifi-no-ble-openthread-endpoint-otns \
    build"

# Build chip-tool controller
./scripts/run_in_build_env.sh \
  "./scripts/build/build_examples.py \
    --target linux-x64-chip-tool-no-ble \
    build"
```

When an OTNS-enabled example binary is launched by OTNS (`otns`), pass `--thread-args` with the OTNS node ID and simulator Unix socket path:

```bash
./out/linux-x64-all-devices-no-wifi-no-ble-openthread-endpoint-otns/all-devices-app \
  --device on-off-light:1 \
  --thread \
  --thread-args "<otns-node-id> <otns-unix-socket-path>"
```

## 2. Running Simulated Home Environments (`otns_all_devices_env.py`)

`scripts/otns/otns_all_devices_env.py` orchestrates an end-to-end simulated Thread home inside the `framichel/otns-matter` container environment (with `/workspace` mounted to the `connectedhomeip` repository root), including:
- Simulated Thread Border Router (`otbr` node + `otbr-agent` + `wpan0` + IPv6 site-local/OMR routing + mDNS proxy)
- Simulated Thread FTD routers (`router` nodes)
- Simulated Matter over Thread nodes running `all-devices-app`
- Automated commissioning, Groupcast (`0x0065`) configuration, wildcard attribute subscriptions, and multicast `On`/`Off` control via `chip-tool`

### Single-Node Verification

```bash
python3 /workspace/scripts/otns/otns_all_devices_env.py --verify
```

### 20-Node Simulated Home with Groupcast & Wildcard Subscriptions

`scripts/otns/otns_20_onoff_group.json` defines a 25-node Thread mesh (1 OTBR Leader, 4 Thread FTD Routers, and 20 Matter `on-off-light` End Devices in a 5x4 grid) configured in Groupcast group `0x0104`:

```bash
# Initial commissioning + Groupcast setup + 1 Group ON/OFF verification cycle
python3 /workspace/scripts/otns/otns_all_devices_env.py \
  --config /workspace/scripts/otns/otns_20_onoff_group.json \
  --verify

# Reuse commissioned state (--reuse-state), establish wildcard subscriptions
# (--subscribe-wildcard), and run 25 Group ON/OFF cycles (50 commands)
python3 /workspace/scripts/otns/otns_all_devices_env.py \
  --config /workspace/scripts/otns/otns_20_onoff_group.json \
  --reuse-state \
  --subscribe-wildcard \
  --group-cycles 25 \
  --group-wait-sec 10 \
  --verify
```

### Evaluating `DelayReportData` (`--delay-report-min-ms` / `--delay-report-jitter-ms`)

To stagger subscription `IM:ReportData` responses after a Groupcast command and avoid 802.15.4 CSMA/CA report storms:

```bash
python3 /workspace/scripts/otns/otns_all_devices_env.py \
  --config /workspace/scripts/otns/otns_20_onoff_group.json \
  --reuse-state \
  --subscribe-wildcard \
  --group-cycles 25 \
  --group-wait-sec 10 \
  --delay-report-min-ms 300 \
  --delay-report-jitter-ms 500 \
  --run-tag 25cycles_delay300_jitter500 \
  --verify
```

## 3. Log Archiving and Performance Analysis Scripts

Each simulation run is automatically archived under `sim_logs/<YYYYMMDD_HHMMSS>_<scenario>_<topology>_<mode>[_<tag>]/` (with `sim_logs/latest` pointing to the most recent run), capturing `manifest.json`, `config.json`, `stats.csv`, `timing_breakdown.txt`, per-node logs (`nodes/`), controller logs (`controller/`), daemon logs (`daemons/`), and 802.15.4 PCAP captures (`pcap/current.pcap`).

- **`scripts/otns/parse_otns_group_timing.py`**: Parses an archived run directory and produces a per-node, per-cycle timing table (`timing_breakdown.txt`) showing MPL multicast reception (`CmdRX`), node `IM:ReportData` transmission (`RepTX`), OTBR forwarding (`OTBR_RX`), and controller reception (`CtrlRX`).
  ```bash
  python3 scripts/otns/parse_otns_group_timing.py sim_logs/latest
  ```
- **`scripts/otns/analyze_otns_group_cdf.py`**: Decrypts 802.15.4 frames in `pcap/current.pcap` and correlates them with node/OTBR/controller logs to compute hop-distance latency distributions and generate CDF plots (`cdf_cmd_rx_by_hops.png`, `cdf_pipeline_stages.png`, `cdf_on_vs_off_and_columns.png`, and `group_latency_stats.json`).
  ```bash
  python3 scripts/otns/analyze_otns_group_cdf.py sim_logs/latest
  ```
- **`scripts/otns/analyze_delay_report_sweep.py`**: Compares multiple `DelayReportData` parameter runs in `sim_logs/`, generating controller `IM:ReportData` latency CDFs (`cdf_delay_report_controller_rx.png`), packet-drop & MAC failure comparisons (`delay_report_packet_drops_comparison.png`), and summary statistics (`delay_report_sweep_stats.json`).
  ```bash
  python3 scripts/otns/analyze_delay_report_sweep.py sim_logs
  ```
