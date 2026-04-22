# Matter (CHIP) Agent Skills

This directory contains skills and instructions for AI agents (Gemini, Co-pilot,
etc.) to assist with development and code review in the Matter repository.

## Available Skills

### Python Test Reviewer

-   **Location**: `.agents/skills/python-test-reviewer/`
-   **Purpose**: Specialized guidance for reviewing Python tests.
-   **Triggers**: Automatically activates when reviewing files ending in
    `_test.py` or located in `src/python_testing`.

## Using Skills

AI agents will automatically discover and load these skills when they are
relevant to your task. You can also explicitly ask the agent to "Use the
python-test-reviewer skill" if you want it to perform a focused review of your
test code.

---

_Note: This structure is designed to support both Gemini and Co-pilot agents.
Core guidelines are being migrated here from `.gemini/styleguide.md`._
