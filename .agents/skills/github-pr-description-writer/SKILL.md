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
use the following sub-headings:

##### Problem

-   Explain the underlying issue that this PR addresses. What is broken,
    missing, or needs improvement?

##### Impact

-   Explain _why_ this is a problem if it is not immediately obvious. What are
    the consequences of not fixing it?

##### Solution

-   Explain the proposed solution and _why_ it fixes the problem.

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
