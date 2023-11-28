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

## Known Issues

-   Python build running out of space
    -   [Fail example](https://github.com/project-chip/connectedhomeip/actions/runs/6239660536/job/16938053552?pr=29333)
    -   May need intermediate cleanup steps

## Tools

-   [Daily Fail Summary](tools/daily_fail_summary.md)

## To Do

-   Cert image auto build
    -   [Initial code](https://github.com/woody-apple/connectedhomeip/tree/build-cert-bins)
    -   Need to change chip-build image builds to cut down time
    -   Do multi-platform builds for ARM64

## General Improvement Ideas
