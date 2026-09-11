# SME Label Reviewers Automation

This automation ensures that pull requests touching specialized domains or
carrying designated GitHub labels receive formal approval from at least one
designated Subject Matter Expert (SME) before merging.

Because GitHub's standard `CODEOWNERS` and branch protection review rules
require dedicated GitHub Enterprise seats for every code owner, this automation
runs as a **standard GitHub Actions status check** (`Check SME Approvals`). It
enforces SME approvals on designated labels with **zero seat license overhead**.

---

## Table of Contents

1. [How to Add a New Label](#1-how-to-add-a-new-label)
2. [How to Manage the YAML Reviewer List](#2-how-to-manage-the-yaml-reviewer-list)
3. [How to Re-Run the Workflow Once Review is Complete](#3-how-to-re-run-the-workflow-once-review-is-complete)
4. [Branch Protection Integration](#4-branch-protection-integration)
5. [Local CLI Verification](#5-local-cli-verification)

---

## 1. How to Add a New Label

You do **not** need to manually create labels on GitHub or have repository admin
permissions.

**New labels are created automatically on GitHub whenever a pull request with
changes to [`.github/label_reviewers.yaml`](./label_reviewers.yaml) is merged
into `master`.**

### Steps to Add a Label:

1. Open a pull request that adds your new label and its designated SME usernames
   to [`.github/label_reviewers.yaml`](./label_reviewers.yaml).
2. Once the PR is reviewed and merged into `master`, GitHub Actions
   automatically detects the new label and creates it in the repository.

_(Optional)_ To have GitHub automatically attach this label to future PRs based
on touched file paths, add matching path rules to
[`.github/labeler.yml`](./labeler.yml):

```yaml
# Example entry in .github/labeler.yml:
security:
    - changed-files:
          - any-glob-to-any-file:
                - "src/crypto/*"
                - "src/crypto/**/*"
```

---

## 2. How to Manage the YAML Reviewer List

The reviewer list is managed in
[`.github/label_reviewers.yaml`](./label_reviewers.yaml).

### File Format

The YAML file is strictly structured as a mapping of label names to lists of
GitHub usernames:

```yaml
<label_name>:
    - <github_username_1>
    - <github_username_2>
```

### Rules & Behaviors

-   **Case-Insensitive**: Both label names and usernames are matched
    case-insensitively (`Security` matches `security`, `Cecille` matches
    `cecille`).
-   **No Leading `@`**: Usernames must not include leading `@` (use `username`,
    not `@username`).
-   **Quotes Optional**: Usernames can be unquoted or enclosed in quotes
    (`username`, `'username'`, or `"username"`). Quotes are not required for
    standard GitHub usernames.
-   **OR-Logic across Labels**: If a PR has multiple designated labels attached
    (e.g., both `security` and `certification`), approval from at least **one**
    reviewer from **any** of the attached labels' lists satisfies the check.
-   **Author Self-Approval Exclusion**: A PR author **cannot approve their own
    PR**. Even if the author is listed as an SME for a label, another reviewer
    from that label's list must provide the approval.
-   **State Transition Awareness**: The check accurately tracks current review
    states. If an SME approves, but later submits `CHANGES_REQUESTED` or the
    approval is `DISMISSED`, the approval is no longer valid until re-approved.

### Example Configuration

```yaml
# Core architecture & SDK infrastructure
core:
    - cecille
    - andy31415

# Security & Cryptography reviews
security:
    - cecille
    - bzbarsky-apple

# Data Model & Cluster XML specifications
data-model:
    - bzbarsky-apple
    - Boris-Virk
```

### Adding or Updating Reviewers

1. Create a branch:
    ```bash
    git checkout -b update-sme-reviewers
    ```
2. Edit [`.github/label_reviewers.yaml`](./label_reviewers.yaml) to add the
   label and username(s).
3. Commit and submit a PR:
    ```bash
    git add .github/label_reviewers.yaml
    git commit -m "Update SME reviewers for <domain>"
    git push origin update-sme-reviewers
    ```
4. Once merged into `master`, all future and open PRs will use the updated
   reviewer list.

---

## 3. How to Re-Run the Workflow Once Review is Complete

Once the designated SME has reviewed and submitted an **APPROVED** review, you
have several quick ways to update the check:

### Method 1: Automatic Re-evaluation (Zero Action Required)

The workflow actively listens to GitHub's `pull_request_review` events
(`submitted`, `edited`, `dismissed`).  
**As soon as the SME submits an "Approved" review, the workflow automatically
re-runs and updates to green without any manual intervention.**

### Method 2: PR Comment Command (`/check-sme`)

You can trigger an immediate re-evaluation directly from the PR conversation
page:

1. Open the PR.
2. Type the following comment and submit:
    ```text
    /check-sme
    ```
3. GitHub Actions will detect the command and trigger the `Check SME Approvals`
   check immediately.

### Method 3: Re-Run from the PR "Checks" Tab

1. On the pull request page, click on the **Checks** tab (or click **Details**
   next to `Check SME Approvals` in the status checks list at the bottom of the
   **Conversation** tab).
2. Select **Check SME Approvals** from the left panel.
3. In the top-right corner, click **Re-run jobs** (or **Re-run**).

### Method 4: Label Toggling

Removing and re-adding the monitored label (or adding any other label) triggers
the `labeled` / `unlabeled` PR event and immediately re-evaluates the check.

---

## 4. Branch Protection Integration

To prevent pull requests from merging until the SME review check passes:

1. Open repository **Settings** -> **Branches**.
2. Under **Branch protection rules**, edit your target branch (e.g. `master`).
3. Check **Require status checks to pass before merging**.
4. In the search box under "Status checks that are required", search for and
   select:
    ```text
    Check SME Approvals
    ```
5. Click **Save changes**.

> [!NOTE] Unlike standard GitHub "Require review from Code Owners", setting
> `Check SME Approvals` as a required status check does **not** consume any
> GitHub Enterprise seats or require external subscription services.

---

## 5. Local CLI Verification

The underlying verification script uses the GitHub CLI (`gh`) and can also be
run locally on a developer workstation or cloudtop:

```bash
# Ensure GitHub CLI is authenticated (via gh auth login or GH_TOKEN)
gh auth status

# Run unit tests
python3 -m unittest scripts/tools/tests/test_check_label_reviewers.py

# Validate YAML configuration syntax and format locally
python3 scripts/tools/check_label_reviewers.py --validate-config

# Run check on a specific PR
python3 scripts/tools/check_label_reviewers.py --pr 30000

# Test with a custom config file
python3 scripts/tools/check_label_reviewers.py --pr 30000 --config /path/to/custom_reviewers.yaml
```
