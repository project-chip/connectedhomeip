#!/usr/bin/env python3
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
# limitations under the License.

"""End-to-end driver for the Media File Management BDX flows.

Launches the Linux tv-app (Media Device / server) and the simplified
tv-casting-app (client), drives the User Directed Commissioning handshake, and
then waits for the three MediaFileManagement demo flows that the simplified app
auto-fires from ConnectionHandler after connecting:

  - AddFile     : client -> tv-app  (tv-app is the BDX receiver / Requestor)
  - OfferFile   : client -> tv-app  (tv-app is the BDX receiver / Requestor)
  - GetSharedFile: tv-app -> client (client is the BDX receiver / Client)

It reuses ProcessOutputCapture from the existing CI harness so the interactive
apps keep a live stdin (otherwise they read EOF and exit immediately).
"""

import logging
import os
import sys
import time

# Reuse the CI harness' process wrapper (holds stdin open, tees output).
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from log_line_processing import ProcessOutputCapture  # noqa: E402

logging.basicConfig(level=logging.INFO, format="%(asctime)s %(message)s")
log = logging.getLogger("mfm_bdx_e2e")

SDK_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
TV_APP = os.path.join(SDK_ROOT, "examples/tv-app/linux/out/debug/chip-tv-app")
CASTING_APP = os.path.join(
    SDK_ROOT, "examples/tv-casting-app/linux/out/simplified/chip-tv-casting-app")

# tv-app operational ports (per examples/tv-app/linux/README.md) so it does not
# collide with the casting-app which also opens an operational listener. Give the
# tv-app its own KVS too, since both apps default to /tmp/chip_kvs.
TV_APP_KVS = "/tmp/chip_kvs_tvapp"
TV_APP_ARGS = ["--secured-device-port", "5640", "--secured-commissioner-port", "5552",
               "--KVS", TV_APP_KVS]

# The simplified casting-app has no --KVS flag; its build overrides the path to
# /tmp/chip_casting_kvs (see the "KVS will be written to" log line at startup).
CLIENT_KVS_GLOB = "/tmp/chip_casting_kvs"

LOG_DIR = "/tmp/mfm-bdx-logs"
TV_APP_LOG = os.path.join(LOG_DIR, "tv-app.log")
CASTING_LOG = os.path.join(LOG_DIR, "casting-app.log")

# Byte-landing checks. The tv-app writes received files under this dir; the
# casting-app writes its GetSharedFile download to this path.
TV_APP_MEDIA_DIR = "/tmp/chip-media-files"
CLIENT_DOWNLOAD = "/tmp/shared-download.bin"
DEMO_CLIP = "/tmp/demo-clip.mp4"


def wait_for(proc, needle, timeout_sec, label):
    """Block until `needle` appears in proc's output, or raise on timeout."""
    deadline = time.time() + timeout_sec
    while time.time() < deadline:
        line = proc.next_output_line(timeout_sec=deadline - time.time())
        if line is None:
            continue
        if needle in line:
            log.info("[%s] matched: %s", label, needle)
            return line
    raise TimeoutError(f"[{label}] timed out waiting for: {needle!r}")


def wait_for_any(proc, needles, timeout_sec, label):
    """Block until any needle appears; returns the (needle, line) that matched."""
    deadline = time.time() + timeout_sec
    remaining = set(needles)
    while time.time() < deadline:
        line = proc.next_output_line(timeout_sec=deadline - time.time())
        if line is None:
            continue
        for n in list(remaining):
            if n in line:
                log.info("[%s] matched: %s", label, n)
                return n, line
    raise TimeoutError(f"[{label}] timed out waiting for any of: {needles}")


def main():
    os.makedirs(LOG_DIR, exist_ok=True)

    # Fresh demo clip + clean landing spots so size checks are meaningful.
    with open(DEMO_CLIP, "wb") as f:
        f.write(bytes((i % 251) for i in range(1024)))
    if os.path.isdir(TV_APP_MEDIA_DIR):
        for f in os.listdir(TV_APP_MEDIA_DIR):
            os.remove(os.path.join(TV_APP_MEDIA_DIR, f))
    if os.path.exists(CLIENT_DOWNLOAD):
        os.remove(CLIENT_DOWNLOAD)

    # Clear persisted fabrics so both apps commission fresh: otherwise the client
    # silently reconnects to a cached fabric and the tv-app never prints the UDC
    # "controller ux ok" prompt the handshake below waits for.
    import glob
    for kvs in glob.glob(CLIENT_KVS_GLOB + "*") + glob.glob(TV_APP_KVS + "*"):
        os.remove(kvs)

    for p in (TV_APP, CASTING_APP):
        if not os.path.exists(p):
            log.error("Missing binary: %s", p)
            return 1

    tv_cmd = ["stdbuf", "-o0", "-i0", TV_APP] + TV_APP_ARGS
    cast_cmd = ["stdbuf", "-o0", "-i0", CASTING_APP]

    with ProcessOutputCapture(tv_cmd, TV_APP_LOG) as tv_app:
        wait_for(tv_app, "Started commissioner", 30, "tv-app")

        with ProcessOutputCapture(cast_cmd, CASTING_LOG) as casting:
            wait_for(casting, "Server initialization complete", 30, "casting")

            # 1. Discovery -> request a cast to player #0.
            wait_for(casting, "Discovered CastingPlayer #0", 30, "casting")
            time.sleep(1)
            casting.send_to_program("cast request 0\n")

            # 2. UDC handshake on the tv-app side.
            wait_for(tv_app, "Via Shell Enter: controller ux ok|cancel", 30, "tv-app")
            tv_app.send_to_program("controller ux ok\n")
            wait_for(tv_app, "Via Shell Enter: controller ux ok [passcode]", 30, "tv-app")
            tv_app.send_to_program("controller ux ok 20202021\n")

            # 3. Connection established -> ConnectionHandler auto-fires the flows.
            wait_for(casting, "Successfully connected to CastingPlayer", 60, "casting")
            log.info("Connected. Waiting for MediaFileManagement BDX flows...")

            # The demo staggers the flows on ~5s timers (AddFile t=0, OfferFile
            # t=5, RequestSharedFiles t=10, GetSharedFile t=15) so the single BDX
            # sender/receiver on each side is never asked to run two at once.

            # 4a. AddFile + OfferFile: tv-app is the BDX receiver. Two Requestor
            # downloads should complete (one per flow).
            wait_for(tv_app, "MediaFileManagementBdxRequestor: download complete", 60,
                     "tv-app AddFile")
            wait_for(tv_app, "MediaFileManagementBdxRequestor: download complete", 60,
                     "tv-app OfferFile")

            # 4b. GetSharedFile: client is the BDX receiver (preceded by
            # RequestSharedFiles which allocates the ResponseID).
            wait_for(casting, "MediaFileManagementBdxClient: download complete", 60,
                     "casting GetSharedFile")

            log.info("All BDX flows reported complete. Letting IO settle...")
            time.sleep(2)

    # 5. Verify bytes landed.
    ok = True
    received = []
    if os.path.isdir(TV_APP_MEDIA_DIR):
        received = [os.path.join(TV_APP_MEDIA_DIR, f)
                    for f in os.listdir(TV_APP_MEDIA_DIR)
                    if f.endswith(".bin")]
    log.info("tv-app received files: %s", received)
    nonempty = [f for f in received if os.path.getsize(f) > 0]
    if len(nonempty) < 2:
        log.error("Expected >=2 non-empty received files (AddFile+OfferFile), got %d",
                  len(nonempty))
        ok = False
    for f in nonempty:
        log.info("  %s : %d bytes", f, os.path.getsize(f))

    if os.path.exists(CLIENT_DOWNLOAD) and os.path.getsize(CLIENT_DOWNLOAD) > 0:
        log.info("client GetSharedFile download: %s : %d bytes",
                 CLIENT_DOWNLOAD, os.path.getsize(CLIENT_DOWNLOAD))
    else:
        log.error("client download %s missing or empty", CLIENT_DOWNLOAD)
        ok = False

    log.info("RESULT: %s", "PASS" if ok else "FAIL")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
