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

"""
Create GitHub issues for test failures that passed on retry (flaky tests).

This script parses failures_yaml.txt to identify tests that failed initially,
then creates GitHub issues with relevant log information for tracking flaky tests.
"""

import argparse
import json
import re
import subprocess
import sys
from pathlib import Path
from typing import Optional, Tuple


def extract_test_case_id(test_name: str) -> Optional[str]:
    """Extract test case ID from test name like 'Test_TC_CGEN_2_2' -> 'TC-CGEN-2.2'."""
    # Pattern: Test_TC_<CLUSTER>_<MAJOR>_<MINOR>
    match = re.search(r'Test_TC_([A-Z]+)_(\d+)_(\d+)', test_name)
    if match:
        cluster = match.group(1)
        major = match.group(2)
        minor = match.group(3)
        return f"TC-{cluster}-{major}.{minor}"
    return None


def extract_step_from_logs(test_name: str, logs: str) -> Tuple[Optional[str], str]:
    """
    Extract step number and failure context from logs.
    
    Returns:
        Tuple of (step_number, log_context)
        step_number can be None if not found
    """
    # Find the test name in logs (case-insensitive)
    test_pattern = re.escape(test_name)
    test_match = re.search(test_pattern, logs, re.IGNORECASE)
    
    if not test_match:
        return None, ""
    
    # Extract context around the test (2000 chars before, 5000 chars after)
    start = max(0, test_match.start() - 2000)
    end = min(len(logs), test_match.end() + 5000)
    context = logs[start:end]
    
    # Look for step numbers in the context
    # Pattern 1: "Test step failure in 'Step 5: ..."
    step_match = re.search(r"Test step failure in ['\"]Step\s+(\d+)[:\s]", context, re.IGNORECASE)
    if step_match:
        return step_match.group(1), context
    
    # Pattern 2: "Step 5:" or "Step 5 " in the context
    step_match = re.search(r'Step\s+(\d+)[:\s]', context, re.IGNORECASE)
    if step_match:
        return step_match.group(1), context
    
    return None, context


def check_existing_issue(test_case_id: str, step_num: str, token: str) -> Optional[int]:
    """Check if an issue already exists for this test failure."""
    title_pattern = f"[Test Failed] [{test_case_id}] Step {step_num}"
    
    # Use gh CLI to search for existing open issues
    try:
        result = subprocess.run(
            ["gh", "issue", "list",
             "--search", f"{title_pattern} in:title is:open",
             "--json", "number,title"],
            capture_output=True,
            text=True,
            env={"GITHUB_TOKEN": token},
            timeout=10
        )
        
        if result.returncode == 0 and result.stdout.strip():
            issues = json.loads(result.stdout)
            if issues:
                return issues[0]["number"]
    except (subprocess.TimeoutExpired, json.JSONDecodeError, Exception) as e:
        print(f"Warning: Could not check for existing issues: {e}", file=sys.stderr)
    
    return None


def create_issue(
    test_case_id: str,
    step_num: str,
    logs: str,
    workflow_run_id: str,
    commit_sha: str,
    repo: str,
    token: str
) -> Optional[str]:
    """Create a GitHub issue for the test failure."""
    title = f"[Test Failed] [{test_case_id}] Step {step_num} fails"
    
    # Check if issue already exists
    existing = check_existing_issue(test_case_id, step_num, token)
    if existing:
        print(f"Issue already exists: #{existing}")
        return f"https://github.com/{repo}/issues/{existing}"
    
    # Limit log size (GitHub issue body has a limit, use 5000 chars)
    log_snippet = logs[:5000] if len(logs) > 5000 else logs
    if len(logs) > 5000:
        log_snippet += "\n\n... (truncated, see workflow logs for full output)"
    
    body = f"""## Test Failure Details

**Test Case:** {test_case_id}
**Failed Step:** Step {step_num}
**Status:** Failed initially but passed on retry (flaky test)

## Workflow Information
- **Workflow Run:** https://github.com/{repo}/actions/runs/{workflow_run_id}
- **Commit:** {commit_sha}
- **Commit Link:** https://github.com/{repo}/commit/{commit_sha}

## Failure Logs
```
{log_snippet}
```

## Next Steps
- [ ] Investigate root cause
- [ ] Add to flaky test tracking
- [ ] Consider test improvements
"""
    
    # Create issue using gh CLI
    try:
        result = subprocess.run(
            ["gh", "issue", "create",
             "--title", title,
             "--body", body,
             "--label", "needs triage",
             "--label", "tests"],
            capture_output=True,
            text=True,
            env={"GITHUB_TOKEN": token},
            timeout=30
        )
        
        if result.returncode == 0:
            issue_url = result.stdout.strip()
            print(f"Created issue: {issue_url}")
            return issue_url
        else:
            print(f"Failed to create issue: {result.stderr}", file=sys.stderr)
            return None
    except subprocess.TimeoutExpired:
        print("Timeout while creating issue", file=sys.stderr)
        return None
    except Exception as e:
        print(f"Error creating issue: {e}", file=sys.stderr)
        return None


def main():
    parser = argparse.ArgumentParser(
        description="Create GitHub issues for test failures that passed on retry"
    )
    parser.add_argument(
        "--failures-file",
        required=True,
        help="Path to failures_yaml.txt"
    )
    parser.add_argument(
        "--logs-file",
        required=True,
        help="Path to test_output.log"
    )
    parser.add_argument(
        "--workflow-run-id",
        required=True,
        help="GitHub workflow run ID"
    )
    parser.add_argument(
        "--commit-sha",
        required=True,
        help="Commit SHA"
    )
    parser.add_argument(
        "--repo",
        required=True,
        help="Repository (owner/repo)"
    )
    parser.add_argument(
        "--github-token",
        help="GitHub token (optional, uses GITHUB_TOKEN env var)"
    )
    
    args = parser.parse_args()
    
    token = args.github_token or sys.environ.get("GITHUB_TOKEN")
    if not token:
        print("Error: GITHUB_TOKEN not provided", file=sys.stderr)
        return 1
    
    failures_file = Path(args.failures_file)
    if not failures_file.exists():
        print("No failures file found, skipping issue creation")
        return 0
    
    logs_file = Path(args.logs_file)
    if not logs_file.exists():
        print("Warning: Logs file not found, skipping issue creation", file=sys.stderr)
        return 0
    
    # Read failed tests
    with open(failures_file) as f:
        failed_tests = [line.strip() for line in f if line.strip()]
    
    if not failed_tests:
        print("No failed tests found")
        return 0
    
    # Read logs
    with open(logs_file) as f:
        logs = f.read()
    
    if not logs:
        print("Warning: No logs available", file=sys.stderr)
        logs = ""
    
    # Process each failed test
    created_issues = []
    skipped_tests = []
    
    for test_name in failed_tests:
        test_case_id = extract_test_case_id(test_name)
        if not test_case_id:
            print(f"Warning: Could not extract test case ID from: {test_name}")
            skipped_tests.append(test_name)
            continue
        
        step_num, test_logs = extract_step_from_logs(test_name, logs)
        if not step_num:
            step_num = "N/A"
        
        # Use extracted context or fallback to general logs
        log_context = test_logs if test_logs else logs[:5000]
        
        issue_url = create_issue(
            test_case_id,
            step_num,
            log_context,
            args.workflow_run_id,
            args.commit_sha,
            args.repo,
            token
        )
        
        if issue_url:
            created_issues.append(issue_url)
    
    # Summary
    if created_issues:
        print(f"\nCreated {len(created_issues)} issue(s):")
        for url in created_issues:
            print(f"  - {url}")
    
    if skipped_tests:
        print(f"\nSkipped {len(skipped_tests)} test(s) (could not extract test case ID):")
        for test in skipped_tests:
            print(f"  - {test}")
    
    return 0


if __name__ == "__main__":
    sys.exit(main())
