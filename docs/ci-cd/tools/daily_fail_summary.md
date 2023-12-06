### Daily Fail Summary

#### Source

Workflow:
https://github.com/project-chip/connectedhomeip/blob/master/.github/workflows/recent_fail_summary.yaml

Script:
https://github.com/project-chip/connectedhomeip/blob/master/scripts/tools/summarize_fail.py

Fail Definitions:
https://github.com/project-chip/connectedhomeip/blob/master/scripts/tools/build_fail_definitions.yaml

#### Summary

Runs once per day; takes inventory of the previous day's workflow runs and
parses them for fail statistics. Creates temporarily cached artifacts for easy
data parsing. Also saves a daily pass percentage list of all workflows at
https://github.com/project-chip/connectedhomeip/blob/daily_pass_percentage/docs/daily_pass_percentage.md.
Fail definitions can be added to the file defined above to allow fast root cause
determination of any fail with an error message.

#### To Do

-   Keep fail signature list updated to track causes of all common fails
-   Include Darwin Test fail definitions in fail summary script - for starters,
    Test Reliable Message Protocol

#### Improvement Ideas

-   Make script artifact more known and accessible so it can be easily shared
    and used by everyone
-   Deliver daily fail summaries in short form through a Slack bot for easy
    access
