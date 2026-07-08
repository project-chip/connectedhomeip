# Matter (CHIP) Agent Skills

This directory contains skills and instructions for AI agents (Gemini, Co-pilot,
etc.) to assist with development and code review in the Matter repository.

## Available Skills

### Python Test Reviewer

-   **Location**: `.agents/skills/python-test-reviewer/`
-   **Purpose**: Specialized guidance for reviewing Python tests.
-   **Triggers**: Automatically activates when reviewing files ending in
    `_test.py` or located in `src/python_testing`.

### GitHub PR Description Writer

-   **Location**: `.agents/skills/github-pr-description-writer/`
-   **Purpose**: Guidelines for generating clear, concise, and objective GitHub
    Pull Request descriptions.
-   **Triggers**: Use when preparing or writing descriptions for pull requests.

### Podman VSCode Build

-   **Location**: `.agents/skills/podman-vscode-build/`
-   **Purpose**: Guidelines for building and testing Matter examples using
    Podman with the official vscode build image in a non-interactive
    environment.
-   **Triggers**: Use when running builds or tests inside a Podman container,
    especially when targeting embedded or cross-compiled platforms.

### GitHub Action Failure Investigation

-   **Location**: `.agents/skills/github-action-failure-investigation/`
-   **Purpose**: Guidelines for fetching, downloading, and analyzing failed
    GitHub Actions runs, including raw logs, packet captures, and detecting
    runner freezes.
-   **Triggers**: Use when investigating a failed GitHub Actions run or job.

## Using Skills

AI agents will automatically discover and load these skills when they are
relevant to your task. You can also explicitly ask the agent to "Use the
python-test-reviewer skill" if you want it to perform a focused review of your
test code.

---

_Note: This structure is designed to support both Gemini and Co-pilot agents.
Core guidelines are being migrated here from `.gemini/styleguide.md`._
