This package contains routines to to collect, aggregate, and report memory
usage, using Pandas `DataFrame` as the primary representation.

-   memdf.df — `DataFrame` utilities, in particular definitions of columns and
    types for the main uses of data frames.
-   memdf.name — Names for synthetic symbols, etc.
-   memdf.collect — Helpers to read memory information from various sources
    (e.g. executables) according to command line options.
-   memdf.select — Helpers to select relevant subsets of data frames according
    to command line or configured options.
-   memdf.report — Helpers to write data frames in various formats according to
    command line or configured options.
-   memdf.util.config — `Config` utility class for managing command line or
    other options according to a declarative description.
-   memdf.util.nd — Nested dictionary utilities, used by `Config`.
-   memdf.util.pretty — Pretty-printed logging utility functions.
-   memdf.util.subprocess — Utilities for executing external commands.
