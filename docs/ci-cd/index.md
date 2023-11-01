# CI/CD Documentation

## Known Issues

-   Python build running out of space

## Tools

### Daily Fail Summary

Workflow:
https://github.com/project-chip/connectedhomeip/blob/master/.github/workflows/recent_fail_summary.yaml

Script:
https://github.com/project-chip/connectedhomeip/blob/master/scripts/tools/summarize_fail.py

Runs once per day; takes inventory of the previous day's workflow runs and parses them for fail statistics. Creates temporarily cached artifacts for easy data parsing. Also saves a daily pass percentage list of all workflows at https://github.com/project-chip/connectedhomeip/blob/daily_pass_percentage/docs/daily_pass_percentage.md

## To Do

-   Cert image auto build

## Improvement Ideas

-   Deliver daily fail summaries in short form through a Slack bot for easy access
