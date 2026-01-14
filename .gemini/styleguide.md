<!--
IMPORTANT: This file should be kept in sync with the copilot instructions file. Any changes made here should be reflected there, and vice-versa, to maintain consistent review instructions for both AI tools.
-->
# Code Review Instructions

- Do not comment on content for XML files or .matter content for clusters
- The SDK is implementing an in-progress matter specification that may not be
  available yet. Assume the matter specification is unknown and out of scope. Do
  not make uninformed assumptions about the Matter specification, or its
  contents.
- Do not comment unless a change is probably desirable
- Do not repeat yourself. Be concise without losing meaning
- Do not over-explain what you see in the code. Only describe code if there are
  specific questions or concerns (or if a question is asked)
- Ensure that extensions or fixes to existing code should match the prevailing
  style of the original code
- Look for common typos and suggest fixes
- Wrong years and years in the future are OK. Humans will catch these issues.
- The SDK uses automated code formatting. Do not comment on whitespace, line
  length or other formatting or whitespace issues. A code formatter will handle
  this.

## Development guides

The SDK source code contains guides for development best practices in
`docs/guides`, `docs/testing` and other `docs` locations. Use these as a
reference for finding common patterns and potential issues in new code. In
particular:

- [docs/guides/writing_clusters](https://github.com/project-chip/connectedhomeip/blob/master/docs/guides/writing_clusters.md)
  describes how cluster handling is to be implemented
- [docs/guides/migrating_ember_cluster_to_code_driven.md](https://github.com/project-chip/connectedhomeip/blob/master/docs/guides/migrating_ember_cluster_to_code_driven.md)
  describes how ember clusters can be migrated to code driven
- [docs/testing/unit_testing.md](https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/unit_testing.md)
  describes how to implement unit testing
- [docs/testing/integration_tests.md](https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/integration_tests.md)
  describes how to implement integration tests.

## Project Health

### Expected Failures in Integration Tests

Tests in `src/python_testing` and `src/app/tests/suites` which verify expected failures
should clearly indicate (either in the test name/label or in a failure output) why the
failure is expected.  For example, if there is a specific section in the spec which
requires the failure, a summary of the section requirements should be included.
