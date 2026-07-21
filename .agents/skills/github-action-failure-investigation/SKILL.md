---
name: github-action-failure-investigation
description:
    Guidelines for fetching, downloading, and analyzing failed GitHub Actions
    runs, including raw logs, packet captures, and detecting runner freezes.
---

# GitHub Action Failure Investigation

This skill provides step-by-step guidelines to retrieve and analyze failed
GitHub Actions runs, check job logs and packet capture artifacts, and classify
failure causes (real bugs vs. racy/frozen runners).

## 1. Prerequisites

Make sure you can run the GitHub CLI (`gh`) and are authenticated:

```bash
gh auth status
```

_Note: If running inside a sandboxed environment that injects dummy tokens
(causing HTTP 401 errors), you may need to prefix commands with
`env -u GITHUB_TOKEN` to force `gh` to fall back to your normal credentials._

## 2. Fetching Job Metadata and Logs

Always create a dedicated staging directory to store logs and artifacts to avoid
cluttering the `out/` root or dirtying the code source tree:

```bash
mkdir -p out/gha/<run_id>/
```

### List Run Status and Artifacts

Use the Run ID from the URL (e.g.,
`https://github.com/project-chip/connectedhomeip/actions/runs/<run_id>`) to
check status and see available artifacts:

```bash
gh run view <run_id>
```

### Download Raw Job Logs

If the run has completed, download the logs for a specific job:

```bash
gh run view <run_id> --job <job_id> --log > out/gha/<run_id>/job_logs.log
```

If the run is still in progress (but the specific job has completed), download
the logs via the GitHub REST API directly:

```bash
gh api repos/project-chip/connectedhomeip/actions/jobs/<job_id>/logs > out/gha/<run_id>/job_logs.log
```

---

## 3. Downloading Capture Files (PCAPs)

If the job summary indicates packet captures were uploaded (e.g.,
`pcaps-linux-repl-TC_[A-C]`), download them to the staging folder:

```bash
gh run download <run_id> -n <artifact_name> --dir out/gha/<run_id>/
```

---

## 4. Investigating Log Failures

1. **Find Failed Tests**: Search or grep for test summary lines to quickly
   identify which test classes failed or had errors:
    - Query: `Summary for test class`
2. **Examine Tracebacks**: Search or grep for tracebacks or error blocks around
   the failed test case:
    - Query: `Traceback (most recent call last)` or `FAIL` or `ERROR`
3. **Inspect Logs**: Open the log file to inspect the context (approx. 100-200
   lines) surrounding the failure line.

---

## 5. Classifying the Root Cause

### A. Real Issues / Bugs

-   Look for C++ assertions, division by zero, null pointer access, or Python
    test assertions (e.g., `AssertionError: True is not False`).
-   Trace back to the source code to locate the logic flaw.

### B. Runner Freeze / Time Compression (Racy/Timing Issues)

GitHub Actions runners (especially ASAN/TSAN runs under heavy load) can freeze
or experience CPU starvation. When the virtual machine is starved and then gets
a burst of CPU, the virtual timers catch up in a rapid burst upon VM resumption
(tick catch-up).

To detect this, compare the **host runner timestamp** (prefix of each log line)
with the **guest application log timestamp** (usually in brackets):

```
Host Runner Timestamp: 2026-06-18T12:18:11.1232136Z [2026-06-18 12:17:58.164368] ...
Host Runner Timestamp: 2026-06-18T12:18:11.1751074Z [2026-06-18 12:18:01.925009] ...
```

-   **General Analysis**:
    -   Compare the host elapsed time and guest elapsed time over a sequence of
        log events.
    -   If a significant amount of guest time (e.g., seconds) passes in a tiny
        fraction of host wall-clock time (e.g., milliseconds), virtual time
        compression is occurring due to CPU starvation/VM freeze.
-   **General Impact of Time Compression**:

    -   Any time-based logic, timers, delays, or backoff protocols (such as
        timeouts, retries, polling loops) will expire/execute instantly or in
        rapid succession when time is compressed.
    -   This leads to false failures, timeouts, session drops, or retry
        exhaustions because the asynchronous event loop doesn't get enough
        wall-clock CPU time to process standard socket queues or network buffers
        before the retry limit is hit.

-   **Example (Case Study: MRP Timeout)**: In the example timestamps above:
    -   **Host elapsed time**: `52 milliseconds`.
    -   **Guest elapsed time**: `3.761 seconds`.
    -   **Result**: The Message Reliability Protocol (MRP) backoff timers
        expired instantly one after another. All retry attempts (4 retries over
        ~4 seconds of virtual time) were sent and exhausted within 52ms of host
        time, causing premature session timeouts before the receiving event loop
        got CPU time to process the socket queue.

---

## 6. Analyzing Packet Capture (PCAP)

Use `tcpdump` to print loopback or network bridge interface packets:

```bash
tcpdump -nn -r out/gha/<run_id>/<pcap_file>
```

Verify the following:

-   Were UDP packets actually transmitted on loopback (`lo In` / `lo Out`)?
-   Did the receiver reply with `StandaloneAck`?
-   If the packets are present in the capture but the receiver logs show no
    `Msg RX` events, the receiver was either frozen, starved of CPU, or its
    virtual time was compressed, preventing it from reading the socket buffer
    before the timeout occurred.
