---
name: github-pr-description-writer
description:
    Guidelines for generating clear, concise, and objective GitHub Pull Request
    descriptions.
---

# Writing GitHub Pull Request Descriptions

This skill provides guidelines for creating clear, concise, and objective pull
request descriptions for the ConnectedHomeIP repository. The goal is to make PRs
easy for human maintainers to understand and review.

## General Rules

These rules apply globally to all sections of the PR description.

### Style and Tone

-   **Objective & Technical**: Write for a technical audience. Use precise
    technical terms.
-   **Concise**: Keep descriptions brief but informative. Avoid walls of text.
    Use bullet points where appropriate.
-   **No Prose or Hype**: Do not use marketing-style language, emojis, or
    subjective/hyperbolic terms (e.g., "amazing", "triumph", "perfect",
    "flawless").
-   **No Filler**: Avoid conversational filler. Get straight to the point.

### Noise Reduction Rule

-   **Say Nothing if Nothing to Say**: Do not include sections or bullet points
    with "None" or "N/A". If a section (like `Caveats` or `#### Related issues`)
    has no content, omit the heading and content entirely to reduce noise.
    _Exception_: The `#### Testing` section must always be present; you may use
    `N/A` there for trivial changes if accompanied by a brief justification.

### Keeping in Sync Rule

-   **Update on Change**: The PR description MUST be updated after every commit
    if the content of the PR materially changes.
    -   **Requires Update**:
        -   Adding or deleting a test.
        -   Changing the implementation approach (e.g., swapping a library
            function or logic).
        -   Adding or removing a feature flag.
    -   **Does NOT Require Update**:
        -   Rebasing or merging with master.
        -   Commit message wording changes only.
        -   Formatting-only changes that do not alter logic.
        -   Minor test execution adjustments (e.g., changing log levels in test
            run) if they don't change what was tested.

## Formatting Guidelines

The PR description must follow a structured format based on the repository's
template, focusing on: **Summary**, **Related Issues**, and **Testing**.

### 1. Summary Section (`#### Summary`)

The summary should explain the "what", "why", and "how" of the changes. It must
use the following sub-headings (where `##### Impact` is optional):

##### Problem

-   Explain the underlying issue that this PR addresses. What is broken,
    missing, or needs improvement?
-   **Optimize for Quick Review**: Use formatting that allows reviewers to
    instantly grasp the problem:
    -   **Structured Breakdown**: Use nested bullets to decompose complex
        problems (e.g., describing a pattern, then listing its consequences like
        **flakiness** or **inefficiency**).
    -   **Bolding for Emphasis**: Highlight key trade-offs, states, or impacts
        using bold text.
    -   **ASCII Diagrams**: If the problem involves a workflow, state machine,
        or timing sequence, include a brief ASCII diagram to visualize it.
    -   If the consequences/impact are clearly explained within this structured
        breakdown, the `##### Impact` section should be omitted to reduce noise.

##### Impact

-   Explain _why_ this is a problem if it is not immediately obvious. What are
    the consequences of not fixing it? Omit if already covered in the structured
    breakdown of the `##### Problem` section.

##### Solution

-   Explain the proposed solution and _why_ it fixes the problem.
-   **Holistic & High-Level**: Focus on the new capabilities, public APIs, or
    architectural changes. Do **NOT** list individual files, classes, or
    internal implementation details (e.g., "Refactored MockX to use dataclass")
    unless they are critically important to the architecture. The diff itself
    shows the file-level changes; the PR description should show the big
    picture.
-   **ASCII Diagrams**: If the solution introduces a new workflow, state
    machine, or architecture, consider including a brief ASCII diagram to
    visualize the new flow.

##### Caveats (Optional)

-   Document any limitations, side effects, performance/memory impacts, or
    specific concerns that reviewers should be aware of. Omit this section
    entirely if there are no caveats.

### 2. Related Issues Section (`#### Related issues`)

-   Reference any related GitHub issues.
-   Use closing keywords (e.g., `Fixes #12345`) if the PR should close the issue
    upon merging.

### 3. Testing Section (`#### Testing`)

Every PR must document how the changes were tested.

-   **Automated Tests**:
    -   If unit tests were added/updated, state so (e.g.,
        `Added unit tests in src/app/tests/...`).
    -   If integration tests (Python/YAML) were used, specify the test names
        (e.g., `Verified with TC_S_2_3.py`).
-   **Manual Testing**:
    -   If manual testing was performed, provide the exact commands and steps to
        reproduce.
    -   Explain _why_ automated testing was not possible.
-   **Trivial Changes**:
    -   If the change is trivial (e.g., documentation typo), it still requires a
        `#### Testing` section. You may use `N/A` with a brief justification.

---

## Example PR Description

Here is an example of a good PR description:

```markdown
#### Summary

Fixes a crash in the DNS-SD client when processing malformed SRV responses.

##### Problem

-   The DNS-SD client does not validate the target host name length in incoming
    SRV records before copying it to a fixed-size buffer.

##### Impact

-   A malformed SRV record with a target host name longer than 63 characters
    causes a buffer overflow, leading to a crash.

##### Solution

-   Added validation to ensure the host name length does not exceed
    `kMaxHostNameLength` before copying.
-   Malformed records are now safely discarded and a warning is logged.

##### Caveats

-   This change discards malformed records and logs a warning.

#### Related issues

Fixes #72327

#### Testing

-   Added unit test `TestDnssdClient_MalformedSrvRecord` in
    `src/lib/dnssd/tests/TestDnssdImpl.cpp` to verify that malformed records are
    discarded and do not cause a crash.
```

### Example 2: Structured Breakdown & Holistic Solution

Here is an example using a structured breakdown with bold highlights for the
problem, and a high-level solution summary:

```markdown
#### Summary

Implemented `WaitForAttributeValue` command in YAML runner to allow tests to
wait for an attribute to reach a specific value, improving test robustness and
reducing execution time. Supported in all Python-based runners.

##### Problem

-   YAML tests lacked polling, requiring a fragile **"trigger transition → sleep
    → verify"** pattern.
-   This fixed-delay pattern is:
    -   **Racy**: Leads to flakiness in slow environments (like CI) if
        transitions take longer than the sleep duration.
    -   **Inefficient**: Forces fast runs to wait for the maximum padded
        duration even if the transition finishes early.

##### Solution

-   Added support for the `WaitForAttributeValue` command to wait for an
    attribute to reach a target value within a timeout, avoiding fixed sleeps.
-   Added the underlying infrastructure to support this across all Python-based
    runners (`chip-tool`, `darwin-framework-tool`, and the REPL runner).
-   Made the extra timeout buffer (slop) configurable (defaults to 250ms
    locally, but configured to 2000ms in GitHub Actions workflows to prevent
    flakiness on slow CI runners).
-   Included a polling history (last 10 attempts with values/errors) in the
    timeout exception message to improve debuggability when a wait fails.

##### Caveats

-   The command uses active polling (100ms interval) to check the attribute
    value.

#### Testing

-   Added integration test
    `src/app/tests/suites/Test_WaitForAttributeValue.yaml` to verify the
    command's timeout behavior.
-   Registered the new test in `src/app/tests/suites/ciTests.json` to enable it
    in CI.
-   Migrated Step 5b of `src/app/tests/suites/certification/Test_TC_S_2_3.yaml`
    to use `WaitForAttributeValue` and verified it passes successfully.
```
