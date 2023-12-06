# CI/CD Documentation

```{toctree}
:glob:

tools/*
```

## Project Information

-   [Build Guide](../guides/BUILDING.md)
-   Sphinx documentation framework
    -   New directories and individual files must be added to the
        [tree](https://github.com/project-chip/connectedhomeip/blob/master/docs/index.md)
    -   New files under directories must be added to the tree in the index file;
        see above. Glob and regular expressions may be used to include all
    -   The
        [documentation page](https://project-chip.github.io/connectedhomeip-doc/)
        is the end product
        -   Links can be relative; links ending in ".md" in the code will be
            reflected as ".html" on that page
-   Pull Requests
    -   Built in style and spelling checks must be satisfied
    -   Larger changes should go through an approval process; reviewers are
        automatically added
    -   Smaller specific changes like ones to this file may be expedited with
        the "fast track" label

Work In Progress

## Tasks

-   [Issues List](https://github.com/project-chip/connectedhomeip/labels/CI%2FCD%20improvements)

## Tools

-   [Daily Fail Summary](tools/daily_fail_summary.md)

## General Improvement Ideas
