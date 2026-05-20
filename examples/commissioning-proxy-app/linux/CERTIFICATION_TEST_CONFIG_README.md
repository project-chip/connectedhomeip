# Commissioning Proxy — Certification Test Rig

This document covers how to **configure and run the Python certification tests**
(`TC_COMPRO_2_*`) for the Commissioning Proxy cluster on a three-RPi test rig,
and how to install the supporting helper scripts on the End Device.

Scope of this document:

-   Wiring up the three-RPi cert-test rig (TH commissioner / Proxy / End
    Device).
-   Installing and configuring the test-runner wrapper
    `~/scripts/run-cert-test.sh` on the TH RPi.
-   Installing the **End-Device cert helpers** that prevent eth0 from masking
    real WiFi-PAF commissioning, plus the watchdog and recovery scripts.
-   All helper scripts are reproduced verbatim below — they are **not** checked
    into the repo. A one-shot installer at the end of this document writes and
    deploys every file to the right host.

Not in scope (covered elsewhere — see references):

-   Hardware list, USB dongle requirements: `../CP_getting_started.md §1`.
-   wpa_supplicant NAN patch (Proxy RPi): `../CP_getting_started.md §2`.
-   Cross-compiling the three binaries: `../CP_getting_started.md §4`.
-   Manually commissioning the proxy / end device:
    `../CP_getting_started.md §6–9`.
-   The Python test list and what each test exercises:
    `IMPLEMENTATION_README.md §Python Certification Tests`.

<hr>

-   [1. Rig topology](#1-rig-topology)
-   [2. One-time TH setup](#2-one-time-th-setup)
    -   [2.1 Python venv and wheels](#21-python-venv-and-wheels)
    -   [2.2 Test scripts and PAA trust store](#22-test-scripts-and-paa-trust-store)
    -   [2.3 Install the test-runner wrapper](#23-install-the-test-runner-wrapper)
-   [3. One-time ED setup](#3-one-time-ed-setup)
    -   [3.1 What the ED cert helpers do](#31-what-the-ed-cert-helpers-do)
    -   [3.2 Install the ED helpers](#32-install-the-ed-helpers)
-   [4. One-time Proxy setup](#4-one-time-proxy-setup)
-   [5. Configure `run-cert-test.sh`](#5-configure-run-cert-testsh)
-   [6. Running tests](#6-running-tests)
-   [7. The eth0 block — what it is, when it engages, how to recover](#7-the-eth0-block--what-it-is-when-it-engages-how-to-recover)
-   [8. One-shot installer](#8-one-shot-installer)
-   [9. Helper scripts (verbatim)](#9-helper-scripts-verbatim)
-   [10. Troubleshooting](#10-troubleshooting)

<hr>

## 1. Rig topology

Three Raspberry Pis on the same LAN, 172.16.62.x is used as an examples (and the
same L2 broadcast domain — see §7 for why this matters):

| Role  | Default IP      | Runs                                                    |
| ----- | --------------- | ------------------------------------------------------- |
| TH    | `172.16.62.53`  | Python tests + `chip-tool`; the **commissioner**        |
| Proxy | `172.16.62.86`  | `chip-commissioning-proxy-app` (patched wpa_supplicant) |
| ED    | `172.16.62.110` | `chip-lighting-app` + cert helper scripts               |

Both Proxy and ED need a NAN-capable USB Wi-Fi dongle (see
`../CP_getting_started.md §1.1`). The on-board Pi Wi-Fi does not support NAN.

> If any of those IPs are different on your rig, edit them in two places: the
> defaults at the top of `~/scripts/run-cert-test.sh` (on the TH) and
> `DEFAULT_COMMISSIONER_IP` in `block-eth0-from-commissioner.sh` (on the ED).

<hr>

## 2. One-time TH setup

### 2.1 Python venv and wheels

The Python test framework runs from a venv with two locally-built Matter wheels
(`matter` core and `matter-clusters`) plus its dependencies. The wheels are
produced by the same SDK build that produces the proxy/ED binaries — see the
wheel build memory note for the full `ninja python_wheels` recipe and the file
transfer sequence.

On the TH RPi:

```bash
sudo apt-get install -y python3-venv python3-dev python3-pip
python3 -m venv ~/matter_venv
source ~/matter_venv/bin/activate
# Copy the two wheels from the build machine first, then:
pip install ~/wheels/matter-*.whl ~/wheels/matter_clusters-*.whl
pip install mobly
```

### 2.2 Test scripts and PAA trust store

The Python tests and a small `compro_support` module run on the TH. Copy them
out of the repo to a working directory:

```bash
mkdir -p ~/matter_tests/support_modules ~/matter_tests/paa-trust-store

# From your build machine
scp src/python_testing/TC_COMPRO_2_*.py \
    ubuntu@<th-ip>:/home/ubuntu/matter_tests/
scp src/python_testing/support_modules/compro_support.py \
    ubuntu@<th-ip>:/home/ubuntu/matter_tests/support_modules/
scp credentials/test/attestation/*.pem \
    ubuntu@<th-ip>:/home/ubuntu/matter_tests/paa-trust-store/
```

### 2.3 Install the test-runner wrapper

`run-cert-test.sh` is the wrapper that orchestrates a single test or the entire
suite. It is reproduced verbatim in §9; the one-shot installer in §8 deploys it
for you.

Manual install:

```bash
mkdir -p ~/scripts
# Paste the contents of §9.5 into ~/scripts/run-cert-test.sh
chmod +x ~/scripts/run-cert-test.sh
```

<hr>

## 3. One-time ED setup

### 3.1 What the ED cert helpers do

Three helpers live on the ED RPi:

| Helper                            | Purpose                                                                                           |
| --------------------------------- | ------------------------------------------------------------------------------------------------- |
| `factory-reset.sh`                | Kill `chip-lighting-app`, clear KVS, drop any WiFi `network={}` blocks left over from a prior run |
| `block-eth0-from-commissioner.sh` | Engage / disengage the eth0 block (see §7) with a self-disarming watchdog                         |
| `unblock-eth0.sh`                 | One-line wrapper around `... down` for keyboard recovery                                          |

In addition the installer drops a login banner
(`/etc/update-motd.d/99-matter-test-rig`) and a NOPASSWD sudoers entry
(`/etc/sudoers.d/matter-test`) so the TH's `EDFixture` can engage the block over
SSH without a password prompt, and a human SSHing in sees a yellow warning if
the block is stuck on.

### 3.2 Install the ED helpers

Easiest: use the one-shot installer in §8 from your workstation.

Manual: paste each script from §9 into the path shown in its header, then:

```bash
chmod +x ~/scripts/{factory-reset,block-eth0-from-commissioner,unblock-eth0}.sh
sudo install -m 755 -o root -g root /tmp/99-matter-test-rig \
    /etc/update-motd.d/99-matter-test-rig
sudo install -m 440 -o root -g root /tmp/sudoers-matter-test \
    /etc/sudoers.d/matter-test
sudo visudo -c -f /etc/sudoers.d/matter-test     # must print "parsed OK"
```

<hr>

## 4. One-time Proxy setup

Nothing cert-test-specific. The proxy needs:

-   The patched wpa_supplicant (`../CP_getting_started.md §2`).
-   The `chip-commissioning-proxy-app` binary in `~/apps/`
    (`../CP_getting_started.md §5`).
-   Password less SSH from the TH for `run-cert-test.sh` to restart the proxy
    between iterations.

`run-cert-test.sh` starts and stops the proxy on every iteration with the flags
configured in §5.

<hr>

## 5. Configure `run-cert-test.sh`

Edit the constants at the top of `~/scripts/run-cert-test.sh` on the TH:

| Variable              | Meaning                                                | Default                               |
| --------------------- | ------------------------------------------------------ | ------------------------------------- |
| `PROXY_IP`            | Proxy RPi IP                                           | `172.16.62.86`                        |
| `PROXY_DISCRIMINATOR` | Discriminator the proxy advertises during its own PASE | `3947`                                |
| `PROXY_PASSCODE`      | Proxy's PASE passcode                                  | `20202021`                            |
| `PROXY_WIFIPAF_FREQ`  | NAN channel(s) for the proxy                           | `2437` (CH 6)                         |
| `PROXY_APP`           | Path to the proxy binary on the proxy RPi              | `~/apps/chip-commissioning-proxy-app` |
| `ED_IP`               | ED RPi IP                                              | `172.16.62.110`                       |
| `ED_APP`              | Path to the lighting app on the ED RPi                 | `~/apps/chip-lighting-app`            |
| `ED_DISCRIMINATOR`    | Discriminator the ED advertises                        | `3840`                                |
| `ED_PASSCODE`         | ED PASE passcode                                       | `20202021`                            |
| `ED_WIFIPAF_FREQ`     | NAN channel(s) for the ED                              | `2437`                                |
| `WIFI_SSID`           | Test WiFi SSID used by TC_COMPRO_2_4 `ConnectNetwork`  | `YOUR_SSID`                           |
| `WIFI_PASSWORD`       | Password for `YOUR_SSID`                               | _set per rig_                         |
| `PAA_TRUST_STORE`     | PAA cert dir on the TH                                 | `~/matter_tests/paa-trust-store`      |
| `STORAGE_PATH`        | chip-tool persistent storage path                      | `/tmp/compro_admin_storage.json`      |
| `ENDPOINT`            | Cluster endpoint                                       | `1`                                   |

Keep `PROXY_WIFIPAF_FREQ == ED_WIFIPAF_FREQ` so NAN can rendezvous.

<hr>

## 6. Running tests

```bash
# Single test, default networked mode, one iteration
bash ~/scripts/run-cert-test.sh 2.4

# Single test, 5 back-to-back iterations
bash ~/scripts/run-cert-test.sh 2.4 5

# Whole suite (2.1 through 2.9), one iteration each, stop on first failure
bash ~/scripts/run-cert-test.sh all -x

# Standalone mode: ED is a physical PAF-only device, operator factory-resets it
bash ~/scripts/run-cert-test.sh 2.4 standalone
```

What happens on every iteration:

1. **Reset.** TH clears its own `/tmp/chip_*` and the proxy's. If the test uses
   the ED, `~/scripts/factory-reset.sh` runs on the ED via SSH.
2. **Proxy start.** Proxy is killed (anchored `pkill`) and re-launched with the
   configured flags.
3. **ED start + eth0 block engaged** (networked mode only). The Python
   `EDFixture` in `compro_support.py` launches the ED over SSH and, _before_ the
   launch, runs `sudo ~/scripts/block-eth0-from-commissioner.sh up` on the ED.
4. **Test runs.** Python `TC_COMPRO_2_X` executes; output is tee to
   `/tmp/matter_run_<test>_<N>.log` on the TH.
5. **ED stop + eth0 block cleared.** `EDFixture.stop()` calls `... block down`
   in a `finally`, so the block is cleared even if the test raises.
6. **Summary.** `run-cert-test.sh` prints a per-test table and an overall
   pass/fail count.

Artifacts:

| Path                                               | Where | Contents                      |
| -------------------------------------------------- | ----- | ----------------------------- |
| `/tmp/matter_run_<test>_<N>.log`                   | TH    | Per-iteration test capture    |
| `/tmp/matter_testing/logs/MatterTest/<timestamp>/` | TH    | Full Matter framework log dir |
| `/tmp/cp_test.log`                                 | Proxy | Proxy stdout/stderr           |
| `/tmp/ed_app.log`                                  | ED    | Lighting-app stdout/stderr    |

<hr>

## 7. The eth0 block — what it is, when it engages, how to recover

### Why the block exists

All three RPis sit on the same `172.16.62.0/24` LAN, and the test WiFi network
(`YOUR_SSID`) is bridged onto the same L2 segment. After a successful
`ConnectNetwork`, the commissioner is supposed to reach the ED on its **WiFi**
IP — but because the ED's eth0 is on the same segment, CASE can silently
establish over eth0 instead, and a bug where WiFi association never happens
still shows green.

### What the block actually drops

`block-eth0-from-commissioner.sh up` installs four iptables-rule layers, all
interface-scoped to eth0:

1. **IPv4 INPUT** from the commissioner IP → `MATTER_BLOCK` chain (ACCEPT
   tcp/22, DROP rest). Commissioner can still SSH; nothing else.
2. **IPv6 INPUT** (any source) → `MATTER_BLOCK` chain (ACCEPT tcp/22 + NDP types
   133/134/135/136, DROP rest). Broad because the commissioner's v6 link-local
   isn't stable.
3. **IPv4 + IPv6 OUTPUT** udp `--sport 5353` **and** `--dport 5353` → DROP.
   Stops mDNS announcements _and_ unicast mDNS responses (which use
   `sport 5353 → requester-port` — a leak that `--dport`-only doesn't catch).
4. SSH from non-commissioner hosts is untouched — the v4 rule is
   `-s <commissioner>`-scoped and the v6 chain ACCEPTs tcp/22 first.

### Watchdog and back-to-back tests

Every `up` call refreshes `/tmp/matter_eth0_block.deadline` to `now + 20 min`
and ensures a singleton background watchdog
(`/tmp/matter_eth0_block.watchdog.pid`) is running. The watchdog ticks every 30
s and runs `... down` once `now > deadline`.

`run-cert-test.sh all` re-arms the deadline on every test's `EDFixture.start()`,
so the watchdog **never fires mid-suite**. It only fires if the test runner dies
and stops refreshing.

Override the window for long debug sessions:

```bash
sudo WATCHDOG_DEADLINE_MIN=120 \
    ~/scripts/block-eth0-from-commissioner.sh up
```

### Recovery from a stuck block

The login banner prints a yellow warning if the block is engaged. To clear it
manually (from the ED keyboard or any SSH-reachable host):

```bash
sudo ~/scripts/unblock-eth0.sh
```

Idempotent — safe to run any time.

### Verifying the block is working

From the TH:

```bash
# eth0 v4 path — should be blocked
ping -c 2 -W 2 <ed-ip>                        # 100% loss

# eth0 v6 link-local — should be blocked (after you find the ED's eth0 LL)
ping6 -c 2 -W 2 -I eth0 fe80::<ed-eth0-LL>    # 100% loss

# eth0 SSH — should still work
ssh ubuntu@<ed-ip> echo ok                     # prints "ok"
```

The post-test ED log should contain
`wpa_supplicant: Interface properties changed, state is 'completed'` — that's
the canary confirming WiFi actually associated, not just an eth0 fall-through:

```bash
ssh ubuntu@<ed-ip> 'grep "state is .completed" /tmp/ed_app.log'
```

<hr>

## 8. One-shot installer

Save this script as `setup-cert-test-rig.sh` **on your workstation** (the
machine that has SSH access to TH/Proxy/ED). It writes every helper to
`/tmp/cert-rig-staging/`, scp them to the right host, sets permissions, installs
the sudoers file with `visudo -c`, and verifies. Re-running it is safe — every
step is idempotent.

```bash
#!/usr/bin/env bash
# setup-cert-test-rig.sh — install Commissioning Proxy cert-test helpers
# Run from a workstation with password less SSH to TH/Proxy/ED.

set -euo pipefail

# ============================================================
# Edit these for your rig.
# ============================================================
TH_HOST=ubuntu@172.16.62.53
ED_HOST=ubuntu@172.16.62.110
# Proxy RPi needs no cert-test helpers; the IP is set in run-cert-test.sh.

STAGE=/tmp/cert-rig-staging
mkdir -p "$STAGE"

# ============================================================
# Write all helpers into the staging dir.
# Copy each block from §9 below verbatim.
# ============================================================

cat > "$STAGE/block-eth0-from-commissioner.sh" <<'BLOCK_EOF'
# >>> paste §9.1 here <<<
BLOCK_EOF

cat > "$STAGE/unblock-eth0.sh" <<'UNBLOCK_EOF'
# >>> paste §9.2 here <<<
UNBLOCK_EOF

cat > "$STAGE/99-matter-test-rig" <<'MOTD_EOF'
# >>> paste §9.3 here <<<
MOTD_EOF

cat > "$STAGE/sudoers-matter-test" <<'SUDOERS_EOF'
# >>> paste §9.4 here <<<
SUDOERS_EOF

cat > "$STAGE/run-cert-test.sh" <<'RUN_EOF'
# >>> paste §9.5 here <<<
RUN_EOF

# ============================================================
# Push to the ED and install.
# ============================================================
echo "=== ED: copy helpers ==="
scp "$STAGE/block-eth0-from-commissioner.sh" \
    "$STAGE/unblock-eth0.sh" \
    "$ED_HOST:/home/ubuntu/scripts/"
scp "$STAGE/99-matter-test-rig" \
    "$STAGE/sudoers-matter-test" \
    "$ED_HOST:/tmp/"

echo "=== ED: chmod / install / validate sudoers ==="
ssh "$ED_HOST" '
    chmod +x ~/scripts/block-eth0-from-commissioner.sh ~/scripts/unblock-eth0.sh &&
    sudo install -m 755 -o root -g root /tmp/99-matter-test-rig \
        /etc/update-motd.d/99-matter-test-rig &&
    sudo install -m 440 -o root -g root /tmp/sudoers-matter-test \
        /etc/sudoers.d/matter-test &&
    sudo visudo -c -f /etc/sudoers.d/matter-test &&
    rm -f /tmp/99-matter-test-rig /tmp/sudoers-matter-test &&
    echo "--- ED files ---" &&
    ls -l ~/scripts/block-eth0-from-commissioner.sh ~/scripts/unblock-eth0.sh \
          /etc/update-motd.d/99-matter-test-rig /etc/sudoers.d/matter-test
'

echo "=== ED: smoke-test up/down ==="
ssh "$ED_HOST" '
    sudo ~/scripts/block-eth0-from-commissioner.sh up &&
    sudo ~/scripts/block-eth0-from-commissioner.sh status &&
    sudo ~/scripts/unblock-eth0.sh
'

# ============================================================
# Push to the TH.
# ============================================================
echo "=== TH: copy run-cert-test.sh ==="
ssh "$TH_HOST" 'mkdir -p ~/scripts'
scp "$STAGE/run-cert-test.sh" "$TH_HOST:/home/ubuntu/scripts/run-cert-test.sh"
ssh "$TH_HOST" 'chmod +x ~/scripts/run-cert-test.sh'

echo "=== done ==="
```

Then run:

```bash
bash setup-cert-test-rig.sh
```

> The script intentionally leaves the helper bodies as `# >>> paste …`
> placeholders so you can audit each script before deploying. Copy each block
> from §9 between the matching delimiters.

<hr>

## 9. Helper scripts (verbatim)

These files are deliberately **not** committed to the repo. Copy each block
verbatim into the path shown in its first comment.

### 9.1 `~/scripts/block-eth0-from-commissioner.sh` (on the ED)

```bash
#!/usr/bin/env bash
# block-eth0-from-commissioner.sh — hide ED's eth0 from the commissioner
# during Matter cert tests so WiFi-association failures are not masked
# by an eth0 fallback path. See ~/README-test-rig.md.
#
# Usage:
#   sudo block-eth0-from-commissioner.sh up   [commissioner_ip]
#   sudo block-eth0-from-commissioner.sh down
#   sudo block-eth0-from-commissioner.sh status
#
# Each "up" extends the watchdog deadline, so back-to-back tests do not
# trigger auto-disarm.  If the test runner dies and stops refreshing the
# deadline, the watchdog runs "down" after WATCHDOG_DEADLINE_MIN minutes.

set -euo pipefail

CHAIN=MATTER_BLOCK
IFACE=eth0
DEFAULT_COMMISSIONER_IP=172.16.62.53

STATE_DIR=/tmp
DEADLINE_FILE=$STATE_DIR/matter_eth0_block.deadline
WATCHDOG_PID_FILE=$STATE_DIR/matter_eth0_block.watchdog.pid
COMMISSIONER_IP_FILE=$STATE_DIR/matter_eth0_block.commissioner_ip

WATCHDOG_DEADLINE_MIN=${WATCHDOG_DEADLINE_MIN:-20}
WATCHDOG_TICK_S=${WATCHDOG_TICK_S:-30}

SCRIPT_PATH=$(readlink -f "$0")

require_root() {
    if [ "$(id -u)" -ne 0 ]; then
        echo "must be run as root (use sudo)" >&2
        exit 1
    fi
}

apply_rules() {
    local ip=$1

    # IPv4 INPUT: drop everything from the commissioner on eth0 except SSH.
    iptables -N "$CHAIN" 2>/dev/null || true
    iptables -F "$CHAIN"
    iptables -A "$CHAIN" -p tcp --dport 22 -j ACCEPT
    iptables -A "$CHAIN" -j DROP
    iptables -C INPUT -i "$IFACE" -s "$ip" -j "$CHAIN" 2>/dev/null \
        || iptables -I INPUT -i "$IFACE" -s "$ip" -j "$CHAIN"

    # IPv6 INPUT: we don't know the commissioner's v6 link-local, so drop
    # all v6 inbound on eth0 except SSH.  Workstation SSH from elsewhere
    # is unaffected (different interface from the ED's perspective, or
    # SSH-port-allowed even if on eth0).
    ip6tables -N "$CHAIN" 2>/dev/null || true
    ip6tables -F "$CHAIN"
    ip6tables -A "$CHAIN" -p tcp --dport 22 -j ACCEPT
    # Allow only NDP (neighbour discovery) so v6 SSH from non-commissioner
    # hosts can still resolve our MAC.  ICMPv6 Echo (ping) stays blocked so
    # diagnosis is unambiguous.
    ip6tables -A "$CHAIN" -p ipv6-icmp --icmpv6-type neighbour-solicitation  -j ACCEPT
    ip6tables -A "$CHAIN" -p ipv6-icmp --icmpv6-type neighbour-advertisement -j ACCEPT
    ip6tables -A "$CHAIN" -p ipv6-icmp --icmpv6-type router-solicitation     -j ACCEPT
    ip6tables -A "$CHAIN" -p ipv6-icmp --icmpv6-type router-advertisement    -j ACCEPT
    ip6tables -A "$CHAIN" -j DROP
    ip6tables -C INPUT -i "$IFACE" -j "$CHAIN" 2>/dev/null \
        || ip6tables -I INPUT -i "$IFACE" -j "$CHAIN"

    # OUTPUT: drop mDNS on eth0 so the commissioner never learns this
    # device's eth0 IP.  Match BOTH --dport 5353 (queries) and
    # --sport 5353 (unicast responses to a non-5353 requester port —
    # this is the common modern case and was the leak path).
    for proto_dir in "iptables --dport" "iptables --sport" \
                     "ip6tables --dport" "ip6tables --sport"; do
        local tool=${proto_dir% *} flag=${proto_dir##* }
        $tool -C OUTPUT -o "$IFACE" -p udp $flag 5353 -j DROP 2>/dev/null \
            || $tool -I OUTPUT -o "$IFACE" -p udp $flag 5353 -j DROP
    done
}

remove_rules() {
    local ip=$1

    iptables  -D INPUT -i "$IFACE" -s "$ip" -j "$CHAIN" 2>/dev/null || true
    iptables  -F "$CHAIN" 2>/dev/null || true
    iptables  -X "$CHAIN" 2>/dev/null || true

    ip6tables -D INPUT -i "$IFACE" -j "$CHAIN" 2>/dev/null || true
    ip6tables -F "$CHAIN" 2>/dev/null || true
    ip6tables -X "$CHAIN" 2>/dev/null || true

    for proto_dir in "iptables --dport" "iptables --sport" \
                     "ip6tables --dport" "ip6tables --sport"; do
        local tool=${proto_dir% *} flag=${proto_dir##* }
        $tool -D OUTPUT -o "$IFACE" -p udp $flag 5353 -j DROP 2>/dev/null || true
    done
}

stop_watchdog() {
    if [ -f "$WATCHDOG_PID_FILE" ]; then
        local pid
        pid=$(cat "$WATCHDOG_PID_FILE" 2>/dev/null || echo "")
        if [ -n "$pid" ] && kill -0 "$pid" 2>/dev/null; then
            kill "$pid" 2>/dev/null || true
        fi
        rm -f "$WATCHDOG_PID_FILE"
    fi
}

start_watchdog() {
    if [ -f "$WATCHDOG_PID_FILE" ]; then
        local pid
        pid=$(cat "$WATCHDOG_PID_FILE" 2>/dev/null || echo "")
        if [ -n "$pid" ] && kill -0 "$pid" 2>/dev/null; then
            return  # already running
        fi
    fi
    setsid bash -c "
        while [ -f '$DEADLINE_FILE' ]; do
            sleep $WATCHDOG_TICK_S
            [ -f '$DEADLINE_FILE' ] || break
            now=\$(date +%s)
            deadline=\$(cat '$DEADLINE_FILE' 2>/dev/null || echo 0)
            if [ \"\$now\" -gt \"\$deadline\" ]; then
                '$SCRIPT_PATH' down
                break
            fi
        done
    " </dev/null >/dev/null 2>&1 &
    echo $! > "$WATCHDOG_PID_FILE"
    disown
}

up() {
    require_root
    local ip=${1:-$DEFAULT_COMMISSIONER_IP}

    # Clear any stale rules first so re-running 'up' is idempotent and a
    # commissioner-IP change between runs does not leak the old rule.
    if [ -f "$COMMISSIONER_IP_FILE" ]; then
        local old_ip
        old_ip=$(cat "$COMMISSIONER_IP_FILE" 2>/dev/null || echo "$ip")
        remove_rules "$old_ip"
    fi
    remove_rules "$ip"

    apply_rules "$ip"
    echo "$ip" > "$COMMISSIONER_IP_FILE"
    date -d "+${WATCHDOG_DEADLINE_MIN} minutes" +%s > "$DEADLINE_FILE"
    start_watchdog
    echo "[ok] eth0 blocked from $ip (deadline $(date -d "@$(cat "$DEADLINE_FILE")"))"
}

down() {
    require_root
    local ip
    ip=$(cat "$COMMISSIONER_IP_FILE" 2>/dev/null || echo "$DEFAULT_COMMISSIONER_IP")
    remove_rules "$ip"
    rm -f "$DEADLINE_FILE" "$COMMISSIONER_IP_FILE"
    stop_watchdog
    echo "[ok] eth0 unblocked"
}

status() {
    require_root
    if iptables -L INPUT -n 2>/dev/null | grep -q "$CHAIN"; then
        local ip
        ip=$(cat "$COMMISSIONER_IP_FILE" 2>/dev/null || echo "?")
        echo "ACTIVE: eth0 blocked from $ip"
        if [ -f "$DEADLINE_FILE" ]; then
            echo "  watchdog deadline: $(date -d "@$(cat "$DEADLINE_FILE")")"
        else
            echo "  no watchdog deadline (will not auto-disarm)"
        fi
        return 0
    else
        echo "INACTIVE: eth0 not blocked"
        return 1
    fi
}

case "${1:-}" in
    up)     shift; up "${1:-}" ;;
    down)   down ;;
    status) status ;;
    *)
        echo "Usage: $0 {up [commissioner_ip] | down | status}" >&2
        exit 2
        ;;
esac
```

### 9.2 `~/scripts/unblock-eth0.sh` (on the ED)

```bash
#!/usr/bin/env bash
# unblock-eth0.sh — convenience wrapper: clear the Matter cert-test eth0
# block in one command.  Safe to run any time; idempotent.

exec /home/ubuntu/scripts/block-eth0-from-commissioner.sh down
```

### 9.3 `/etc/update-motd.d/99-matter-test-rig` (on the ED, mode 0755 root:root)

```sh
#!/bin/sh
# Login banner: warn if the Matter cert-test eth0 block is still engaged.
# Checks the state file (world-readable) rather than iptables so it works
# for unprivileged logins.

if [ -f /tmp/matter_eth0_block.commissioner_ip ]; then
    ip=$(cat /tmp/matter_eth0_block.commissioner_ip 2>/dev/null)
    printf '\n\033[1;33m*** Matter cert-test eth0 block is ACTIVE ***\033[0m\n'
    printf '*** Commissioner %s cannot reach this host via eth0 (SSH still OK).\n' "${ip:-?}"
    printf '*** To clear: sudo /home/ubuntu/scripts/unblock-eth0.sh\n\n'
fi
```

### 9.4 `/etc/sudoers.d/matter-test` (on the ED, mode 0440 root:root)

Install with `sudo visudo -c -f /etc/sudoers.d/matter-test` to validate.

```
# /etc/sudoers.d/matter-test
# Allow the ubuntu user to run the cert-test eth0 block helpers without
# password, so EDFixture (running over SSH from the commissioner) can
# arm/disarm the block.  Limited to these two scripts only.
ubuntu ALL=(root) NOPASSWD: /home/ubuntu/scripts/block-eth0-from-commissioner.sh, /home/ubuntu/scripts/unblock-eth0.sh
```

### 9.5 `~/scripts/run-cert-test.sh` (on the TH)

```bash
#!/usr/bin/env bash

# ============================================================
# Configuration — edit these for your setup
# ============================================================
PROXY_IP=172.16.62.86
PROXY_DISCRIMINATOR=3947
PROXY_PASSCODE=20202021
PROXY_WIFIPAF_FREQ=2437
PROXY_APP=/home/ubuntu/apps/chip-commissioning-proxy-app
PROXY_LOG=/tmp/cp_test.log

ED_IP=172.16.62.110
ED_APP=/home/ubuntu/apps/chip-lighting-app
ED_DISCRIMINATOR=3840
ED_PASSCODE=20202021
ED_WIFIPAF_FREQ=2437

WIFI_SSID=<YOUR_SSID>
WIFI_PASSWORD=<Password>

PAA_TRUST_STORE=~/matter_tests/paa-trust-store
STORAGE_PATH=/tmp/compro_admin_storage.json
ENDPOINT=1

# ============================================================
# Helpers
# ============================================================

proxy_ssh() {
    ssh -o StrictHostKeyChecking=no -o BatchMode=yes -n ubuntu@"${PROXY_IP}" "$@"
}

ed_ssh() {
    ssh -o StrictHostKeyChecking=no -o BatchMode=yes -n ubuntu@"${ED_IP}" "$@"
}

factory_reset_all() {
    echo "--- Factory reset Commissioner"
    rm -f /tmp/chip_*
    echo "--- Factory resetting proxy on ${PROXY_IP} ---"
    proxy_ssh "rm -f /tmp/chip_*"
    echo ""
}

# Reset the ED before a test run.  In networked mode the ED is SSH-reachable
# and factory-reset.sh is run remotely.  In standalone mode the operator is
# prompted to reset the physical device manually.
reset_ed() {
    if [[ "${ED_MODE}" == "networked" ]]; then
        echo "--- Factory resetting ED on ${ED_IP} ---"
        ed_ssh "~/scripts/factory-reset.sh"
    else
        read -r -p "--- Standalone ED: please factory reset the ED and press Enter to continue --- " _
    fi
    echo ""
}

# Run a single test invocation, capturing output for step analysis.
# Usage: run_test <name> <run> <total> [extra args...]
#   run/total are used only for the header; pass 1 1 for a non-repeated test.
run_test() {
    local name="$1"
    local run="$2"
    local total="$3"
    shift 3

    if (( total > 1 )); then
        echo "=== ${name} [run ${run}/${total}] ==="
    else
        echo "=== ${name} ==="
    fi

    local capture="/tmp/matter_run_${name}_${run}.log"
    local rc elapsed fail_step status

    local t0=$SECONDS
    python3 "${name}.py" \
        --commissioning-method on-network \
        --discriminator "${PROXY_DISCRIMINATOR}" \
        --passcode     "${PROXY_PASSCODE}" \
        --storage-path "${STORAGE_PATH}" \
        --paa-trust-store-path "${PAA_TRUST_STORE}" \
        --endpoint     "${ENDPOINT}" \
        --tests        "test_${name}" \
        "$@" 2>&1 | tee "${capture}"
    rc=${PIPESTATUS[0]}
    elapsed=$(( SECONDS - t0 ))

    status=$([ $rc -eq 0 ] && echo PASS || echo FAIL)

    fail_step="-"
    if [[ $status == FAIL ]]; then
        fail_step=$(grep -oE '\*\*\*\*\* Test Step [0-9]+' "${capture}" | \
                    tail -1 | grep -oE '[0-9]+')
        [[ -z $fail_step ]] && fail_step="?"
    fi

    RESULT_NAMES+=("$name")
    RESULT_RUNS+=("${run}/${total}")
    RESULT_STATUS+=("$status")
    RESULT_TIMES+=("${elapsed}s")
    RESULT_FAIL_STEPS+=("$fail_step")
    echo ""
}

# Run a test RUNS times, performing a full factory reset before each run.
# Accepts --with-ed to also factory-reset the ED (networked SSH or operator prompt).
# Accepts --stop-on-fail/-x to abort after the first failure.
# Usage: run_test_multi <name> <runs> [--with-ed] [extra args...]
run_test_multi() {
    local name="$1"
    local runs="$2"
    shift 2

    local with_ed=0
    local extra_args=()
    for arg in "$@"; do
        if [[ "$arg" == "--with-ed" ]]; then
            with_ed=1
        else
            extra_args+=("$arg")
        fi
    done

    for (( run=1; run<=runs; run++ )); do
        factory_reset_all
        if [[ "${with_ed}" == "1" ]]; then
            reset_ed
        fi
        reset_proxy
        run_test "$name" "$run" "$runs" "${extra_args[@]}"
        if [[ "${STOP_ON_FAIL}" == "1" && "${RESULT_STATUS[-1]}" == "FAIL" ]]; then
            echo "--- stop-on-fail: aborting after failed run ${run}/${runs} ---"
            break
        fi
    done
}

# If --stop-on-fail is active and any test has already failed, exit immediately.
# The EXIT trap fires first, so the summary is always printed before exit.
stop_if_failed() {
    [[ "${STOP_ON_FAIL}" == "1" ]] || return 0
    printf '%s\n' "${RESULT_STATUS[@]}" | grep -q "^FAIL$" || return 0
    exit 1
}

reset_proxy() {
    echo "--- Resetting proxy on ${PROXY_IP} ---"
    proxy_ssh "kill -9 \$(pgrep -f chip-commissioning-proxy-app 2>/dev/null) 2>/dev/null; true"
    sleep 1
    proxy_ssh "rm -f /tmp/chip_*"
    proxy_ssh "nohup ${PROXY_APP} --wifi --wifipaf freq_list=${PROXY_WIFIPAF_FREQ} --discriminator ${PROXY_DISCRIMINATOR} > ${PROXY_LOG} 2>&1 < /dev/null &"
    sleep 4
    proxy_ssh "pgrep -f chip-commissioning-proxy-app > /dev/null && echo Proxy running OK || { echo ERROR: proxy failed to start; exit 1; }"
    rm -f "${STORAGE_PATH}"
    echo ""
}

print_summary() {
    [[ ${#RESULT_NAMES[@]} -eq 0 ]] && return

    # Determine whether any test had multiple runs
    local max_total=1
    local r
    for r in "${RESULT_RUNS[@]}"; do
        local t="${r#*/}"
        (( t > max_total )) && max_total=$t
    done

    local multi=$(( max_total > 1 ))

    echo "============================================================"
    echo "  Test Summary"
    echo "============================================================"

    if (( multi )); then
        printf "  %-22s  %-7s  %-6s  %-6s  %s\n" "Test" "Run" "Result" "Time" "Failed Step"
        echo "  --------------------------------------------------------"
    else
        printf "  %-22s  %-6s  %s\n" "Test" "Result" "Time"
        echo "  ----------------------------------------------------"
    fi

    local all_pass=0 all_fail=0
    local i name run status time fstep

    # Ordered list of unique test names (preserving first-seen order)
    local -a name_order
    local -A seen_names test_pass_count test_fail_count test_fail_step_list test_total_secs

    for i in "${!RESULT_NAMES[@]}"; do
        name="${RESULT_NAMES[$i]}"
        run="${RESULT_RUNS[$i]}"
        status="${RESULT_STATUS[$i]}"
        time="${RESULT_TIMES[$i]}"
        fstep="${RESULT_FAIL_STEPS[$i]}"

        # Per-run row
        if (( multi )); then
            local step_col=""
            [[ $fstep != "-" ]] && step_col="step $fstep"
            printf "  %-22s  %-7s  %-6s  %-6s  %s\n" \
                "$name" "$run" "$status" "$time" "$step_col"
        else
            printf "  %-22s  %-6s  %s\n" "$name" "$status" "$time"
        fi

        # Aggregate tracking
        if [[ -z ${seen_names[$name]+x} ]]; then
            seen_names[$name]=1
            name_order+=("$name")
            test_pass_count[$name]=0
            test_fail_count[$name]=0
            test_fail_step_list[$name]=""
            test_total_secs[$name]=0
        fi

        (( test_total_secs[$name] += ${time%s} ))

        if [[ $status == PASS ]]; then
            (( all_pass++ ))
            (( test_pass_count[$name]++ ))
        else
            (( all_fail++ ))
            (( test_fail_count[$name]++ ))
            if [[ $fstep != "-" ]]; then
                if [[ -n ${test_fail_step_list[$name]} ]]; then
                    test_fail_step_list[$name]+=",$fstep"
                else
                    test_fail_step_list[$name]="$fstep"
                fi
            fi
        fi
    done

    echo "  --------------------------------------------------------"

    if (( multi )); then
        for name in "${name_order[@]}"; do
            local p=${test_pass_count[$name]}
            local f=${test_fail_count[$name]}
            local total_runs=$(( p + f ))
            local steps_note=""
            local steps="${test_fail_step_list[$name]}"
            [[ -n $steps ]] && steps_note="  (failed at step: ${steps})"
            local avg=$(( test_total_secs[$name] / total_runs ))
            printf "  %-22s  %d/%d passed  avg %ds%s\n" "${name}:" "$p" "$total_runs" "$avg" "$steps_note"
        done
        echo "  --------------------------------------------------------"
        printf "  Overall: %d passed  %d failed  of %d\n" \
            "$all_pass" "$all_fail" "$(( all_pass + all_fail ))"
    else
        printf "  Passed: %d   Failed: %d   Total: %d\n" \
            "$all_pass" "$all_fail" "$(( all_pass + all_fail ))"
    fi

    echo "============================================================"
    printf "  ED logs (on device)   : /tmp/ed_app.log\n"
    printf "  CP logs (on device)   : /tmp/cp_test.log\n"
    if (( multi )); then
        printf "  Run captures (local)  : /tmp/matter_run_<test>_<N>.log\n"
    fi
    printf "  Commissioner logs     : /tmp/matter_testing/logs/MatterTest/latest/\n"
    echo "============================================================"
    echo ""
}

# ============================================================
# Main
# ============================================================
declare -a RESULT_NAMES RESULT_STATUS RESULT_TIMES RESULT_RUNS RESULT_FAIL_STEPS
trap print_summary EXIT

cd ~/matter_tests
source ~/matter_venv/bin/activate

# Strip --stop-on-fail / -x from positional args before further parsing.
STOP_ON_FAIL=0
FILTERED=()
for arg in "$@"; do
    case "$arg" in
        --stop-on-fail|-x) STOP_ON_FAIL=1 ;;
        *) FILTERED+=("$arg") ;;
    esac
done
set -- "${FILTERED[@]}"

TEST="${1:-}"

# Accept:  ./run-cert-test.sh 2.4 [N]
#          ./run-cert-test.sh 2.4 [networked|standalone] [N]
# If $2 looks like a positive integer, treat it as RUNS (ED_MODE defaults to networked).
if [[ "${2:-}" =~ ^[1-9][0-9]*$ ]]; then
    ED_MODE="networked"
    RUNS="${2}"
else
    ED_MODE="${2:-networked}"
    RUNS="${3:-1}"
fi

case "${ED_MODE}" in
    networked|standalone) ;;
    *)
        echo "ERROR: unknown ED mode '${ED_MODE}' — expected 'networked' or 'standalone'"
        exit 1
        ;;
esac

if ! [[ "$RUNS" =~ ^[1-9][0-9]*$ ]]; then
    echo "ERROR: run count must be a positive integer, got '${RUNS}'"
    exit 1
fi

# Build the ED arg array based on the selected mode.
# networked: pass app path + SSH host so the test launches/stops the ED automatically.
# standalone: pass only discriminator/passcode; the test will prompt the operator.
if [[ "${ED_MODE}" == "networked" ]]; then
    ED_ARGS=(
        --string-arg "ed_app_path:${ED_APP}" "ed_ssh_host:${ED_IP}"
        --string-arg "ed_extra_args:--wifi --wifipaf freq_list=${ED_WIFIPAF_FREQ}"
        --int-arg "ed_discriminator:${ED_DISCRIMINATOR}" "ed_passcode:${ED_PASSCODE}"
    )
else
    ED_ARGS=(
        --int-arg "ed_discriminator:${ED_DISCRIMINATOR}" "ed_passcode:${ED_PASSCODE}"
    )
fi

case "${TEST}" in
    2.1)
        run_test_multi TC_COMPRO_2_1 "$RUNS"
        ;;
    2.2)
        [[ $RUNS -gt 1 ]] && echo "--- Running ${RUNS} times, ED mode: ${ED_MODE} ---"
        run_test_multi TC_COMPRO_2_2 "$RUNS" --with-ed "${ED_ARGS[@]}"
        ;;
    2.3)
        [[ $RUNS -gt 1 ]] && echo "--- Running ${RUNS} times, ED mode: ${ED_MODE} ---"
        run_test_multi TC_COMPRO_2_3 "$RUNS" --with-ed "${ED_ARGS[@]}"
        ;;
    2.4)
        [[ $RUNS -gt 1 ]] && echo "--- Running ${RUNS} times, ED mode: ${ED_MODE} ---"
        run_test_multi TC_COMPRO_2_4 "$RUNS" --with-ed \
            --string-arg "wifi_ssid:${WIFI_SSID}" "wifi_password:${WIFI_PASSWORD}" \
            "${ED_ARGS[@]}"
        ;;
    2.5)
        run_test_multi TC_COMPRO_2_5 "$RUNS"
        ;;
    2.6)
        [[ $RUNS -gt 1 ]] && echo "--- Running ${RUNS} times, ED mode: ${ED_MODE} ---"
        run_test_multi TC_COMPRO_2_6 "$RUNS" --with-ed "${ED_ARGS[@]}"
        ;;
    2.7)
        [[ $RUNS -gt 1 ]] && echo "--- Running ${RUNS} times, ED mode: ${ED_MODE} ---"
        run_test_multi TC_COMPRO_2_7 "$RUNS" --with-ed "${ED_ARGS[@]}"
        ;;
    2.8)
        [[ $RUNS -gt 1 ]] && echo "--- Running ${RUNS} times, ED mode: ${ED_MODE} ---"
        run_test_multi TC_COMPRO_2_8 "$RUNS" --with-ed "${ED_ARGS[@]}"
        ;;
    2.9)
        run_test_multi TC_COMPRO_2_9 "$RUNS"
        ;;
    all)
        [[ $RUNS -gt 1 ]] && echo "--- Running each test ${RUNS} times, ED mode: ${ED_MODE} ---"
        run_test_multi TC_COMPRO_2_1 "$RUNS"
        stop_if_failed
        run_test_multi TC_COMPRO_2_2 "$RUNS" --with-ed "${ED_ARGS[@]}"
        stop_if_failed
        run_test_multi TC_COMPRO_2_3 "$RUNS" --with-ed "${ED_ARGS[@]}"
        stop_if_failed
        run_test_multi TC_COMPRO_2_4 "$RUNS" --with-ed \
            --string-arg "wifi_ssid:${WIFI_SSID}" "wifi_password:${WIFI_PASSWORD}" \
            "${ED_ARGS[@]}"
        stop_if_failed
        run_test_multi TC_COMPRO_2_5 "$RUNS"
        stop_if_failed
        run_test_multi TC_COMPRO_2_6 "$RUNS" --with-ed "${ED_ARGS[@]}"
        stop_if_failed
        run_test_multi TC_COMPRO_2_7 "$RUNS" --with-ed "${ED_ARGS[@]}"
        stop_if_failed
        run_test_multi TC_COMPRO_2_8 "$RUNS" --with-ed "${ED_ARGS[@]}"
        stop_if_failed
        run_test_multi TC_COMPRO_2_9 "$RUNS"
        ;;
    *)
        echo "Usage: $0 [2.1|2.2|2.3|2.4|2.5|2.6|2.7|2.8|2.9|all] [networked|standalone] [N] [--stop-on-fail|-x]"
        echo "       $0 [2.1|2.2|2.3|2.4|2.5|2.6|2.7|2.8|2.9|all] [N] [--stop-on-fail|-x]"
        echo ""
        echo "  networked      ED is an RPi accessible via SSH at ${ED_IP} (default)"
        echo "  standalone     ED is a physical WiFiPAF-only device; test will prompt operator"
        echo "  N              number of times to run each test (default: 1)"
        echo "  --stop-on-fail stop iterating after the first failed run (-x is a shorthand)"
        exit 1
        ;;
esac

# When --stop-on-fail triggered an early abort, exit non-zero so callers can
# detect failure even though print_summary always runs and exits 0.
[[ "${STOP_ON_FAIL}" == "1" ]] && printf '%s\n' "${RESULT_STATUS[@]}" \
    | grep -q "^FAIL$" && exit 1 || true
```

### 9.6 `~/scripts/factory-reset.sh` (on the ED)

Already present on most rigs; reproduced here for completeness. Adjust `IFACE`
to match your USB Wi-Fi dongle and `~/script/config_paf_env.sh` to whatever
PAF-environment script your dongle vendor ships with.

```bash
#!/usr/bin/env bash
set -e

IFACE="wlx44a56e5961cf"
WPA_CONF="/etc/wpa_supplicant/wpa_supplicant.conf"
CHIP_LIGHTING_APP="chip-lighting-app"

clear
rm /tmp/chip_* || true

echo "Terminate Matter example application ${CHIP_LIGHTING_APP}"
ps aux | grep -E "${CHIP_LIGHTING_APP}" | grep -v grep || true
killall -q ${CHIP_LIGHTING_APP} || true

echo "Removing network blocks..."
sudo sed -i '/network={/,/}/d' "$WPA_CONF"

cd ~/script
~/script/config_paf_env.sh comee
echo "Wi-Fi configuration cleared."
echo "Current interface status:"
ip a show "$IFACE"
```

<hr>

## 10. Troubleshooting

### "Cannot reach ED" / SSH from TH stalls

If `run-cert-test.sh` hangs at "Factory resetting ED" the SSH from TH is being
dropped. The eth0 block exempts tcp/22 from the commissioner, so the likely
culprits are:

-   Block engaged for a different commissioner IP. Check
    `sudo cat /tmp/matter_eth0_block.commissioner_ip` on the ED — if it doesn't
    match the TH, run `sudo ~/scripts/unblock-eth0.sh` and re-arm with the
    correct IP.
-   Workstation IP changed between deploy and run. The TH IP is the only one the
    block treats specially; SSH from any other host always works.

### Test passes but no `state is 'completed'` in `/tmp/ed_app.log`

The eth0 fallback masked a real WiFi-association failure. Verify the block was
actually engaged during the test by checking the iteration's start banner —
`EDFixture.start()` will have run `... block up` over SSH. Also re-check the
four sanity probes in §7 ("Verifying the block is working").

### Step 6 (`CommissionViaProxy`) intermittently times out

Known PAFTP handshake-data readiness race — the `EDFixture._start_remote`
already sleeps 5 s after launching the ED to give wpa_supplicant's NAN context
time to settle, but the race still surfaces occasionally. Re-run the test; the
second iteration almost always passes. This is unrelated to the eth0 block.

### `sudo` prompts for a password during a test run

The NOPASSWD sudoers entry didn't get installed or didn't validate. On the ED:

```bash
sudo visudo -c -f /etc/sudoers.d/matter-test       # must print "parsed OK"
ls -l /etc/sudoers.d/matter-test                    # must be 0440 root:root
```

If the file is missing, re-run the installer in §8.
