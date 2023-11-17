# CI/CD Documentation

```{toctree}
:glob:

tools/*
```

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
