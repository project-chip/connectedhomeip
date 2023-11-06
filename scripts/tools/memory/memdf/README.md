---
orphan: true
---

This package contains routines to to collect, aggregate, and report memory
usage, using Pandas `DataFrame` as the primary representation.

-   memdf.collect — Helpers to read memory information from various sources
    (e.g. executables) according to command line options.
-   memdf.df — `DataFrame` utilities, in particular definitions of columns and
    types for the main uses of data frames.
-   memdf.name — Names for synthetic symbols, etc. Individual readers are
    located under memdf.collector.
-   memdf.report — Helpers to write data frames in various formats according to
    command line or configured options.
-   memdf.select — Helpers to select relevant subsets of data frames according
    to command line or configured options.
-   memdf.sizedb — Helpers for a database of size information.

Modules under memdf.util are not specifically tied to memory usage.

-   memdf.util.config — `Config` utility class for managing command line or
    other options according to a declarative description.
-   memdf.util.github — Utilities for communicating with GitHub.
-   memdf.util.markdown — Utilities for manipulating Markdown text.
-   memdf.util.nd — Nested dictionary utilities, used by `Config`.
-   memdf.util.pretty — Pretty-printed logging utility functions.
-   memdf.util.sqlite - Utilities for connecting to a sqlite3 database.
-   memdf.util.subprocess — Utilities for executing external commands.
