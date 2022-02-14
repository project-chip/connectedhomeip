# Memory usage scripts

Scripts to collect, aggregate, and report memory usage.

## Common options

The following options are common to _most_ of the scripts, where applicable:

### optional arguments:

-   `-h`, `--help` Show this help message and exit.
-   `--verbose`, `-v` Show informational messages; repeat for debugging
    messages.
-   `--config-file` _FILE_ Read configuration _FILE_. Typically this is a file
    from the `platform/` subdirectory providing platform-specific option
    defaults. Command line options override the configuration file.

### input options:

-   `--collect-method` _METHOD_, `-f` _METHOD_ Method of input processing.

    This specifies how the input files are read. The available *METHOD*s are:

    -   `elftools` — use the python elftools library
    -   `readelf` — use the external `readelf` program
    -   `bloaty` — use the external `bloaty` program
    -   `csv` — read a comma-separated table
    -   `tsv` — read a tab-separated table
    -   `su` — read `.su` stack usage reports

    Not all methods are usable for all scripts. Usually `readelf` is fastest.

-   `--collect-prefix` _PATH_, `--prefix` _PATH_, `--strip-prefix` _PATH_ Strip
    _PATH_ from the beginning of source file names.

## output options:

-   `--output-file` _FILENAME_, `--output` _FILENAME_, `-O` _FILENAME_ Output
    file. Defaults to standard output. For `csv` and `tsv` formats, this is
    actually an output file name prefix.
-   `--output-format` _FORMAT_, `--to` _FORMAT_, `-t` _FORMAT_ Output format.
    One of:
    -   `text` — Plain text tables, in a single file.
    -   `csv` — Comma-separated tables (in several files, if not stdout).
    -   `tsv` — Tab-separated tables (in several files, if not stdout).
    -   `json_split` — JSON - see Pandas documentation for details.
    -   `json_records` — JSON - see Pandas documentation for details.
    -   `json_index` — JSON - see Pandas documentation for details.
    -   `json_columns` — JSON - see Pandas documentation for details.
    -   `json_values` — JSON - see Pandas documentation for details.
    -   `json_table` — JSON - see Pandas documentation for details.
    -   Any format provided by [tabulate](https://pypi.org/project/tabulate/).
-   `--report-limit` _BYTES_, `--limit` _BYTES_ Limit display to items above the
    given size. Suffixes (e.g. `K`) are accepted.
-   `--report-by` _GROUP_, `--by` _GROUP_ Reporting group. One of:
    -   `region` — Aggregate by region. A ‘region’ is a platform-defined memory
        area consisting of a number of segments. Commonly platforms define the
        regions `FLASH` and `RAM`.
    -   `section` — Aggregate by section.
    -   `symbol` — Aggregate by symbol name.

## selection options:

-   `--section-select` _NAME_, `--section` _NAME_ Section(s) to process;
    otherwise all not ignored. Note: platform configuration files typically
    define a default list of sections.
-   `--section-select-all` Select all sections.
-   `--section-ignore` _NAME_ Section(s) to ignore.
-   `--section-ignore-all` Ignore all sections unless explicitly selected.
-   `--symbol-select` _NAME_, `--symbol` _NAME_ Symbol(s) to process; otherwise
    all not ignored.
-   `--symbol-select-all` Select all symbols.
-   `--symbol-ignore` _NAME_ Symbol(s) to ignore.
-   `--symbol-ignore-all` Ignore all symbols unless explicitly selected.
-   `--region-select` _NAME_, `--region` _NAME_ Region(s) to process; otherwise
    all not ignored.
-   `--region-select-all` Select all regions.
-   `--region-ignore` _NAME_ Region(s) to ignore.
-   `--region-ignore-all` Ignore all regions unless explicitly selected.

## external tool options:

-   `--tool-bloaty` _FILE_ File name of the bloaty executable.
-   `--tool-nm` _FILE_ File name of the nm executable.
-   `--tool-readelf` _FILE_ File name of the readelf executable.

## Scripts

### report_summary.py

Report the total size of each region or section (per `--report-by`).

Example:

```
$ report_summary.py --by=region --config-file=${PLATFORM}.cfg ${IMAGE}
   region   size
*unknown*    200
FLASH     524285
RAM       165501
```

### report_tree.py

Present a tree-structured report of memory use.

For this script, `--limit` (or a per-section limit) is useful.

Example:

```
$ report_tree.py --demangle --by=region --region=RAM:8K \
    --prefix=${CHIP_TOOLS} --prefix=${PWD} \
    --config-file=${PLATFORM}.cfg \
    ${IMAGE}

REGION: RAM
100% 62540 *total*
├── 32% 19896 WS_vlatest
├── 28% 17671 out
│   └── 100% 17671 release
│       └── 100% 17671 ..
│           └── 100% 17671 ..
│               ├── 96% 16911 third_party
│               │   ├── 100% 16866 lwip
│               │   │   └── 100% 16866 repo
│               │   │       └── 100% 16866 lwip
│               │   │           └── 100% 16866 src
│               │   └──  0% 45 *other*
│               └──  4% 760 *other*
└── 40% 24973 *other*
```

### gaps.py

Report parts of an image that are not defined as part of any symbol. Typically
these are string constants or other anonymous data.

**Note**: currently this only works with the `elftools` reader and consequently
only works on ELF files.

Example:

```

$ gaps.py --section=.text --limit=1K --config-file=${PLATFORM}.cfg ${IMAGE}
04065992 length 5482 in section .text of ${IMAGE}
04065990:       54 43 50 00 55 44 50 00 47 6F 74 20 6D 6F TCP.UDP.Got mo
040659A0: 72 65 20 41 43 4B 65 64 20 62 79 74 65 73 20 28 re ACKed bytes (
040659B0: 25 64 29 20 74 68 61 6E 20 77 65 72 65 20 70 65 %d) than were pe
040659C0: 6E 64 69 6E 67 20 28 25 64 29 00 47 6F 74 20 41 nding (%d).Got A
040659D0: 43 4B 20 66 6F 72 20 25 64 20 62 79 74 65 73 20 CK for %d bytes
...

```

### diffsyms.py

Reports differences in size (and/or presence) of individual symbols between two
files. Generally this only makes sense between different versions of the same
thing, e.g. between a build on a working branch vs master.

Example:

```
$ diffsyms.py --demangle ${IMAGE1} ${IMAGE2}
                                                                                         symbol   a   b
chip::Inet::InetLayer::NewUDPEndPoint(chip::Inet::UDPEndPoint**)                                196 194
chip::Transport::BLE::Init(chip::DevicePairingDelegate*, chip::RendezvousParameters const&)  80 100
```

### block.py

Report symbol references found in a block list. Generally useful only on library
(`.a`) or object files.

Example:

```

$ block.py --config-file=blocklist.cfg out/release/\${PLATFORM}/obj/src/transport/lib/libTransportLayer.a

address kind symbol                   cu
      0 U    strcpy TransportMgrBase.cpp
```

### collect.py

Read memory use and write it in another form.

Useful for example to capture symbols from an image and write them to file(s) in
some useful format (e.g. csv, json) for further processing.
