# CI/CD Documentation

## Known Issues
- Cert image autobuild
- Python build running out of space

## Tools
### Fail Summarization
Workflow: https://github.com/project-chip/connectedhomeip/blob/master/.github/workflows/recent_fail_summary.yaml

Script: https://github.com/project-chip/connectedhomeip/blob/master/scripts/tools/summarize_fail.py

To Do:
- 24 hour pass/fail rate for all workflows
- Write output to a .md file
- Make fail list a separate editable .yml
