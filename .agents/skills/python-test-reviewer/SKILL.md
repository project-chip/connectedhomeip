---
name: python-test-reviewer
description:
    Expert guidance for reviewing Python tests in the Matter (connectedhomeip)
    repository. Use this skill when reviewing changes to tests, specifically
    targeting common pitfalls in async execution, mocking cluster interactions,
    and assertion quality.
---

# Python Test Reviewer Skill

This skill provides a checklist and set of principles for reviewing Python tests
within the Matter repository.

## Core Review Principles

-   **Prefer Real Interactions Over Mocks**: For integration tests, prioritize
    using the real Matter stack (via fixtures) unless the interaction is too
    complex or slow.
-   **Explicit Assertions**: Never accept tests that "pass by not crashing."
    Every test must have at least one clear, meaningful assertion.
-   **Async Hygiene**: Matter is heavily asynchronous. Ensure async patterns are
    used correctly.

## Specific Checkpoints

### 1. Asyncio Usage

-   **Check**: Are all awaitable calls (like `devCtrl.ReadAttribute`) actually
    awaited?
-   **Propagate Exceptions in Parallel Code**: When using concurrent.futures or
    similar parallel execution mechanisms, ensure that exceptions thrown in
    background threads are not silently ignored. Call .result() on futures to
    propagate errors.

### 2. Mocking Cluster Interactions

-   **Guideline**: When mocking cluster responses, ensure the mock structure
    matches the `chip.clusters` attributes or commands exactly.
-   **Pitfall**: Avoid mocking the entire `ChipDeviceCtrl` if you only need to
    mock a single attribute read.

### 3. Code Style

-   **Re-use**: Avoid re-writing code that is present in matter/testing or the
    chip core libraries. If you are creating a class or function that is
    substantially similar to existing classes or functions, consider whether
    extension would be preferable to duplication
-   **Attribute reads**: Prefer using read_single_attribute_check_success in
    matter/testing to the base Read and ReadAttribute functions in
    ChipDeviceCtrl.py unless there is a good reason. This function contains
    additional checks that are useful for testing.
-   **Exceptions**: Avoid `except Exception` - use a specific exception
-   **try / except can hide bugs**: Avoid fixing CI issues with a try: except:.
    This pattern can hide real bugs that should be fixed and is a code smell.
    Try except blocks should only be used where the exception is expected and
    that should normally include an assertion that the exception happens or a
    comment explaining in detail why the exception is thrown and how this is
    spec compliant.
-   **Sleeps can hide bugs**: a sleep in a test is a code smell. Ensure sleeps
    in tests are actually required. If the test is using sleep to wait for the
    device to do something, consider whether a subscription can be used instead.
-   **Tolerances should be well considered**: If there is a fudge factor built
    in, ensure the logic is both explained and reasonable.
-   **Use dataclass**: Do not use dicts with string keys. Use dataclasses with
    named members instead. This helps programmers avoid uncaught typos.
-   **Avoid Decorative Separators**: Remove long lines of hashes (e.g.,
    ###########) or other decorative separators. Use standard spacing or
    docstrings instead.
-   **Cleanup of Stale Code**: Remove outdated TODO comments, unbound variables,
    and leftover debugging code.
-   **Type Hints**: Use type hints for function signatures to improve
    readability and maintainability, especially for functions that are part of
    the test framework or are likely to be reused.
-   **TODOs**: TODOs should be linked to a tracking issue and used sparingly.

### 4. Matter-Specific Patterns

-   **Commissioning**: If the test involves commissioning, ensure it uses the
    standard commissioning fixtures to avoid duplicated setup logic.
-   **IDs**: IDs and values should be taken from the codegen files (ex.
    Objects.py). Values that are not in the codegen should be flagged for follow
    up

### 5. Testing failures

-   **Test for unexpected successes**: If a test is meant to verify that an
    error condition is properly handled, ensure that it fails if the error
    condition does not occur.

## Feedback Style

-   Be concise.
-   Reference specific lines.
-   If a pattern is repeated across many tests, suggest a helper function or
    fixture.
