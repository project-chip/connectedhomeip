# Matter Versioning and Release Maintenance

**Silicon Labs Matter GitHub is being phased out in favor of using the Matter GSDK Extension that is available through Simplicity Studio and standalone via SLC-CLI.**

## Versioning Scheme

- **FORMAT: vMAJOR.MINOR.PATCH-CSA\_VERSION-PRE\_RELEASE**

- Silicon Labs versions e.g., 3.0.0

- CSA Matter version e.g., 1.1

- [Optional] Prerelease flag e.g., alpha.1

**Example:** v3.0.0-1.1-alpha.1

Update digits based on the following criteria:

- **Major**

  - CSA ups the major version number, or

  - Major GSDK version supported, or

  - Miscellaneous breaking change

- **Minor**

  - CSA ups the minor version number, or

  - Minor GSDK version supported, or

  - Silabs updates e.g., new sample app, documentation, bug fixes, or

  - Silabs hardware platform added, or

- **Patch**

  - Patch GSDK version supported, or

  - Targeted bug fixes

- **Pre-Release**

  - As needed, e.g. release after TE1 can be called v2.3.0-1.2-alpha.1

    - To identify targeted feature requests/SQA quality level testing

      - alpha level quality to identify non-standard SQA level testing

      - beta level quality to identify certain SQA testing, but still in early development


## Release Maintenance

### Silabs Matter Github (SMG) 

| **Release** | **Date** | **GSDK** | **WiSeConnect 3 SDK** | **Status** |
|-------------|----------|----------|----------|----------|
| v0.1.0	| 26-July-22	| v4.1.0 | N/A | Obsolete |
| v0.2.0	| 17-Aug-22	| v4.1.0	| N/A | Obsolete |
| v0.3.0	| 8-Sept-22	| v4.1.1 | 	N/A | Obsolete |
| v0.4.0	| 13-Oct-22	| v4.1.1	| N/A | Obsolete |
| v1.0.0	| 2-Nov-22	| v4.1.1	| N/A | Obsolete |
| v1.0.2-1.0	| 19-Dec-22	| v4.1.1	| N/A | Monitored |
| v1.1.0-1.1	| 24-Feb-23	| v4.2.0	| N/A | Monitored |
| v2.0.0-1.1	| 18-May-23	| v4.2.0	| N/A | Obsolete |
| v2.1.0-1.1	| 23-June-23	| v4.2.3	| N/A | Monitored |
| v2.2.0-1.2-alpha-1	| 30-Aug-23	| v4.3.1	| v3.0.10 | Obsolete |
| v2.2.0-1.2	| 25-Oct-23	| v4.3.2	| v3.1.0 | Monitored |
| v2.3.0-1.3-alpha.1	| 18-Jan-24	| v4.4.0	| v3.1.1 | Unmaintained |
| v2.3.0-1.3-alpha.2	| 15-Feb-24	| v4.4.1	| v3.1.3 | Unmaintained |
| v2.3.0-1.3-beta.1	| 18-Mar-24	| v4.4.1	| v3.1.3.2 | Unmaintained |
| v2.3.0-1.3	| 22-Apr-24	| v4.4.2	| v3.1.3.4 | Active |
