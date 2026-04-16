#!/usr/bin/env python3

import argparse
import json
import math
import os
from collections import defaultdict
from datetime import datetime, timezone, timedelta
from typing import Any, Dict, List


GOOD_STATUSES = {"passed"}
BAD_STATUSES = {"failed", "broken"}
KNOWN_STATUSES = {"passed", "failed", "broken", "skipped", "unknown"}


def iso_week_key(dt: datetime) -> str:
    year, week, _ = dt.isocalendar()
    return f"{year}-W{week:02d}"


def month_key(dt: datetime) -> str:
    return f"{dt.year}-{dt.month:02d}"


def safe_int(value: Any, default: int = 0) -> int:
    try:
        return int(value)
    except Exception:
        return default


def normalize_test_name(full_name: str) -> str:
    # Input example:
    # "REPL Tests Linux - master:REPL Tests Linux - master#TC_BOOLCFG_6_1.py"
    # Keep the right-most token after '#', else after ':', else whole string.
    if "#" in full_name:
        return full_name.split("#")[-1]
    if ":" in full_name:
        return full_name.split(":")[-1]
    return full_name


def summarize_test(full_name: str, payload: Dict[str, Any]) -> Dict[str, Any]:
    items = payload.get("items", []) or []

    weekly = defaultdict(lambda: {
        "runs": 0, "passed": 0, "failed": 0, "broken": 0, "skipped": 0, "unknown": 0, "nonPassing": 0
    })
    monthly = defaultdict(lambda: {
        "runs": 0, "passed": 0, "failed": 0, "broken": 0, "skipped": 0, "unknown": 0, "nonPassing": 0
    })

    status_counts = {
        "passed": 0,
        "failed": 0,
        "broken": 0,
        "skipped": 0,
        "unknown": 0,
    }

    durations: List[int] = []
    normalized_runs: List[Dict[str, Any]] = []

    now = datetime.now(timezone.utc)
    cutoff_7d = now - timedelta(days=7)
    cutoff_30d = now - timedelta(days=30)

    last7d = {
        "runs": 0,
        "passed": 0,
        "failed": 0,
        "broken": 0,
        "skipped": 0,
        "unknown": 0,
        "nonPassing": 0,
    }

    last30d = {
        "runs": 0,
        "passed": 0,
        "failed": 0,
        "broken": 0,
        "skipped": 0,
        "unknown": 0,
        "nonPassing": 0,
    }

    for item in items:
        status = item.get("status", "unknown")
        if status not in KNOWN_STATUSES:
            status = "unknown"

        time_info = item.get("time", {}) or {}
        start_ms = safe_int(time_info.get("start"))
        stop_ms = safe_int(time_info.get("stop"))
        duration_ms = safe_int(time_info.get("duration"))

        if duration_ms <= 0 and stop_ms > start_ms:
            duration_ms = stop_ms - start_ms

        if start_ms <= 0:
            continue

        dt = datetime.fromtimestamp(start_ms / 1000.0, tz=timezone.utc)
        week = iso_week_key(dt)
        month = month_key(dt)

        weekly[week]["runs"] += 1
        weekly[week][status] += 1
        monthly[month]["runs"] += 1
        monthly[month][status] += 1

        if status in BAD_STATUSES:
            weekly[week]["nonPassing"] += 1
            monthly[month]["nonPassing"] += 1

        status_counts[status] += 1

        if duration_ms > 0:
            durations.append(duration_ms)

        normalized_runs.append({
            "uid": item.get("uid"),
            "status": status,
            "start": start_ms,
            "stop": stop_ms,
            "duration": duration_ms,
            "reportUrl": item.get("reportUrl"),
        })

        # last 7 days
        if dt >= cutoff_7d:
            last7d["runs"] += 1
            last7d[status] += 1
            if status in BAD_STATUSES:
                last7d["nonPassing"] += 1

        # last 30 days
        if dt >= cutoff_30d:
            last30d["runs"] += 1
            last30d[status] += 1
            if status in BAD_STATUSES:
                last30d["nonPassing"] += 1

    normalized_runs.sort(key=lambda r: r["start"], reverse=True)

    total_runs = len(normalized_runs)
    passed_runs = status_counts["passed"]
    failed_runs = status_counts["failed"]
    broken_runs = status_counts["broken"]
    skipped_runs = status_counts["skipped"]
    unknown_runs = status_counts["unknown"]
    non_passing_runs = failed_runs + broken_runs

    avg_duration = round(sum(durations) / len(durations)) if durations else 0
    pass_rate = round((passed_runs / total_runs) * 100.0, 2) if total_runs else 0.0
    failure_rate = round((non_passing_runs / total_runs) * 100.0, 2) if total_runs else 0.0

    last_run = normalized_runs[0] if normalized_runs else None

    def sort_bucket_dict(d: Dict[str, Dict[str, int]]) -> Dict[str, Dict[str, int]]:
        return {k: d[k] for k in sorted(d.keys())}

    def finalize_window_stats(window: Dict[str, int]) -> Dict[str, Any]:
        runs = window["runs"]
        passed = window["passed"]
        non_passing = window["nonPassing"]
        return {
            **window,
            "passRate": round((passed / runs) * 100.0, 2) if runs else 0.0,
            "failureRate": round((non_passing / runs) * 100.0, 2) if runs else 0.0,
            "flaky": passed > 0 and non_passing > 0,
        }

    return {
        "fullName": full_name,
        "displayName": normalize_test_name(full_name),
        "totalRuns": total_runs,
        "passedRuns": passed_runs,
        "failedRuns": failed_runs,
        "brokenRuns": broken_runs,
        "skippedRuns": skipped_runs,
        "unknownRuns": unknown_runs,
        "nonPassingRuns": non_passing_runs,
        "passRate": pass_rate,
        "failureRate": failure_rate,
        "avgDurationMs": avg_duration,
        "lastRun": last_run["start"] if last_run else None,
        "lastStatus": last_run["status"] if last_run else None,
        "lastReportUrl": last_run["reportUrl"] if last_run else None,
        "weekly": sort_bucket_dict(weekly),
        "monthly": sort_bucket_dict(monthly),
        "last7d": finalize_window_stats(last7d),
        "last30d": finalize_window_stats(last30d),
        "recentRuns": normalized_runs[:100],
    }


def build_summary(raw: Dict[str, Any]) -> Dict[str, Any]:
    tests = []
    for full_name, payload in raw.items():
        if not isinstance(payload, dict):
            continue
        tests.append(summarize_test(full_name, payload))

    tests.sort(key=lambda t: (-t["nonPassingRuns"], t["displayName"].lower()))

    total_tests = len(tests)
    total_runs = sum(t["totalRuns"] for t in tests)
    total_failed = sum(t["failedRuns"] for t in tests)
    total_broken = sum(t["brokenRuns"] for t in tests)
    total_skipped = sum(t["skippedRuns"] for t in tests)
    total_passed = sum(t["passedRuns"] for t in tests)
    total_non_passing = total_failed + total_broken

    worst_by_failures = sorted(
        tests,
        key=lambda t: (-t["nonPassingRuns"], -t["totalRuns"], t["displayName"].lower())
    )[:20]

    worst_by_rate = sorted(
        [t for t in tests if t["totalRuns"] >= 3],
        key=lambda t: (-t["failureRate"], -t["nonPassingRuns"], t["displayName"].lower())
    )[:20]

    return {
        "generatedAt": datetime.now(timezone.utc).isoformat(),
        "summary": {
            "totalTests": total_tests,
            "totalRuns": total_runs,
            "passedRuns": total_passed,
            "failedRuns": total_failed,
            "brokenRuns": total_broken,
            "skippedRuns": total_skipped,
            "nonPassingRuns": total_non_passing,
            "passRate": round((total_passed / total_runs) * 100.0, 2) if total_runs else 0.0,
        },
        "highlights": {
            "worstByFailures": [
                {
                    "fullName": t["fullName"],
                    "displayName": t["displayName"],
                    "nonPassingRuns": t["nonPassingRuns"],
                    "failureRate": t["failureRate"],
                    "totalRuns": t["totalRuns"],
                }
                for t in worst_by_failures
            ],
            "worstByRate": [
                {
                    "fullName": t["fullName"],
                    "displayName": t["displayName"],
                    "nonPassingRuns": t["nonPassingRuns"],
                    "failureRate": t["failureRate"],
                    "totalRuns": t["totalRuns"],
                }
                for t in worst_by_rate
            ],
        },
        "tests": tests,
    }


def main() -> None:
    parser = argparse.ArgumentParser(description="Build a static dashboard summary from Allure-like history.json")
    parser.add_argument("--input", required=True, help="Path to history.json")
    parser.add_argument("--output", required=True, help="Path to summary.json")
    args = parser.parse_args()

    with open(args.input, "r", encoding="utf-8") as f:
        raw = json.load(f)

    summary = build_summary(raw)

    os.makedirs(os.path.dirname(os.path.abspath(args.output)), exist_ok=True)
    with open(args.output, "w", encoding="utf-8") as f:
        json.dump(summary, f, indent=2, sort_keys=False)

    print(f"Wrote {args.output}")


if __name__ == "__main__":
    main()