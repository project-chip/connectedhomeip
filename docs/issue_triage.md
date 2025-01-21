# Project issues

General issues are listed at
<https://github.com/project-chip/connectedhomeip/issues> .

In order to be able to effectively follow up on these issues, they are separated
into groups for further review and fixing based on current maintainers on
specific areas of the code. The separation is done via `labels`.

Issues that still need triaging are labeled as `needs triage`:

<https://github.com/project-chip/connectedhomeip/issues?q=is%3Aissue+is%3Aopen+label%3A%22needs+triage%22>

## Non-SDK issues

In some cases the issue is integrating with ecosystems or environments are not
supported. In these cases, issues can be re-directed to specific support pages
rather than the SDK GitHub issues.

### Google Integration

-   General integration link: <https://developers.home.google.com/matter>
-   Support via the communities areas:
    -   Stack overflow:
        <https://stackoverflow.com/questions/tagged/google-smart-home>
    -   Google Nest Community:
        <https://www.googlenestcommunity.com/t5/Smart-Home-Developer-Forum/bd-p/Smart-Home-Developer-Forum>

### Apple Integration

This applies if there is an issue with Home interacting with a device (as
opposed to an issue with Matter.framework or the SDK that is encountered by code
actually running on darwin).

General instructions available at [here](./guides/darwin.md) in the
`Providing feedback to Apple` section.

Once a Feedback Assistant ticket is filed, the ticket ID can be sent to Boris
Zbkarsky to ensure it is noticed by the right people

### Environments not currently supported / not maintained

Some items are explicitly not maintained currently

-   Windows integration: no official maintainer for this so currently not
    supported
-   Old compiler support: CHIP requires a C++17 or higher compiler.

## Platform maintainers

This level of separation is generally for platform-specific issues (e.g. failure
to commission for one specific platform, failure to run on some specific
operating system).

Contact is generally done on slack. E-mail addresses are not added here on
purpose in order to avoid spam.

| Platform     | Contact                                 | Label                                                                                                    | Note(s) |
| ------------ | --------------------------------------- | -------------------------------------------------------------------------------------------------------- | ------- |
| Android      | Andrei Litvin, Yunhan Wang, Yufeng Wang | [android](https://github.com/project-chip/connectedhomeip/issues?q=is%3Aopen+is%3Aissue+label%3Aandroid) |         |
| Darwin       | Boris Zbarsky, Justin Wood              | [darwin](https://github.com/project-chip/connectedhomeip/issues?q=is%3Aopen+is%3Aissue+label%3Adarwin)   |         |
| Espressif    | Hrishikesh Dhayagude                    | [esp32](https://github.com/project-chip/connectedhomeip/issues?q=is%3Aopen+is%3Aissue+label%3Aesp32)     |         |
| Linux        | Andrei Litvin                           | [linux](https://github.com/project-chip/connectedhomeip/issues?q=is%3Aopen+is%3Aissue+label%3Alinux)     |         |
| Nordic       | Lucasz Duda                             | [nrf](https://github.com/project-chip/connectedhomeip/issues?q=is%3Aopen+is%3Aissue+label%3Anrf)         |         |
| NXP          | Doru Gucea                              | [nxp](https://github.com/project-chip/connectedhomeip/issues?q=is%3Aopen+is%3Aissue+label%3Anxp)         |         |
| Silabs/EFR32 | Jean Francois Penven, Junior Martinez   | [efr32](https://github.com/project-chip/connectedhomeip/issues?q=is%3Aopen+is%3Aissue+label%3Aefr32)     |         |

## Code areas

The following people can be contacted about issues in specific area of code are
affected that are not platform-specific.

| Code area                             | Contact                          | Label                                                                                            | Note(s)                                                                                                                                                                                    |
| ------------------------------------- | -------------------------------- | ------------------------------------------------------------------------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| ARM cross-compile                     | Andrei Litvin                    | N/A                                                                                              | Some support for cross-compiling on x64 linux for arm64 linux (like Raspberry PI)                                                                                                          |
| Build system (darwin, xcode)          | Boris Zbarsky, Justin Wood       | N/A                                                                                              | darwin specific builds, xcode connector                                                                                                                                                    |
| Build system (gn, ninja)              | Andrei Litvin                    | N/A                                                                                              | General build system questions and gn support, generally on Linux                                                                                                                          |
| MatterIDL                             | Andrei Litvin                    | N/A                                                                                              | General .matter files and code generation based on it                                                                                                                                      |
| Minimal MDNS                          | Andrei Litvin                    | N/A                                                                                              | mdns specfic. Note that platformdns also exists and issues are often "DNSSD" rather than minmdns specific                                                                                  |
| Python testing infrastructure/helpers | Cecille Freeman                  | N/A                                                                                              | Writing tests cases in python. **NOTE** this is for test infrastrure and NOT for individual test issues. Test case failures are associated with test applications or the test case script. |
| ZAP Code Generation                   | Boris Zbarsky, Bharat Raju Dandu | [zap](https://github.com/project-chip/connectedhomeip/issues?q=is%3Aopen+is%3Aissue+label%3Azap) | Some issues may be in the zap project itself. This is for generic code generation issues and help, often regarding `.zapt` templates                                                       |
| src/crypto, src/credentials           | Tennessee Carmel-Veilleux        | N/A                                                                                              |                                                                                                                                                                                            |

## Example maintenance

Examples often correspond to specific device types. We have split the contact
per device type regardless of example (e.g. all-clusters will contain all device
types or functionality) as well as individual examples.

### Per device type

| Device Type(s)             | Contact                      | Label                                                                                                        | Note(s) |
| -------------------------- | ---------------------------- | ------------------------------------------------------------------------------------------------------------ | ------- |
| Fabric Bridge, Fabric Sync | Yufeng Wang, Terence Hampson | [fabric-sync](https://github.com/project-chip/connectedhomeip/pulls?q=is%3Aopen+is%3Apr+label%3Afabric-sync) |         |

### Per example

| Example path                                   | Contact                      | Note(s)      |
| ---------------------------------------------- | ---------------------------- | ------------ |
| `examples/air-purifier-app`                    |                              | UNMAINTAINED |
| `examples/air-quality-sensor-app`              |                              | UNMAINTAINED |
| `examples/all-clusters-app`                    |                              | UNMAINTAINED |
| `examples/all-clusters-minimal-app`            |                              | UNMAINTAINED |
| `examples/android`                             | Yunhan Wang                  |              |
| `examples/bridge-app`                          |                              | UNMAINTAINED |
| `examples/build_overrides`                     |                              | UNMAINTAINED |
| `examples/chef`                                | Andrei Litvin                |              |
| `examples/chip-tool`                           |                              | UNMAINTAINED |
| `examples/contact-sensor-app`                  |                              | UNMAINTAINED |
| `examples/darwin-framework-tool`               | Boris Zbarsky                |              |
| `examples/dishwasher-app`                      |                              | UNMAINTAINED |
| `examples/energy-management-app`               | James Harrow                 |              |
| `examples/fabric-admin`                        | Yufeng Wang, Terence Hampson |              |
| `examples/fabric-bridge-app`                   | Yufeng Wang, Terence Hampson |              |
| `examples/java-matter-controller`              | Yunhan Wang, Yufeng Wang     |              |
| `examples/kotlin-matter-controller`            | Yunhan Wang, Yufeng Wang     |              |
| `examples/laundry-washer-app`                  |                              | UNMAINTAINED |
| `examples/lighting-app`                        | Junior Martinez              |              |
| `examples/lighting-app-data-mode-no-unique-id` |                              | UNMAINTAINED |
| `examples/light-switch-app`                    |                              | UNMAINTAINED |
| `examples/lit-icd-app`                         | Yunhan Wang                  |              |
| `examples/lock-app`                            |                              | UNMAINTAINED |
| `examples/log-source-app`                      |                              | UNMAINTAINED |
| `examples/microwave-oven-app`                  | Rob Bultman                  |              |
| `examples/minimal-mdns`                        | Andrei Litvin                |              |
| `examples/network-manager-app`                 | Thomas Lea                   |              |
| `examples/ota-provider-app`                    |                              | UNMAINTAINED |
| `examples/ota-requestor-app`                   |                              | UNMAINTAINED |
| `examples/persistent-storage`                  |                              | UNMAINTAINED |
| `examples/pigweed-app`                         |                              | UNMAINTAINED |
| `examples/placeholder`                         |                              | UNMAINTAINED |
| `examples/providers`                           |                              | UNMAINTAINED |
| `examples/pump-app`                            |                              | UNMAINTAINED |
| `examples/pump-controller-app`                 |                              | UNMAINTAINED |
| `examples/refrigerator-app`                    |                              | UNMAINTAINED |
| `examples/resource-monitoring-app`             |                              | UNMAINTAINED |
| `examples/rvc-app`                             |                              | UNMAINTAINED |
| `examples/smoke-co-alarm-app`                  |                              | UNMAINTAINED |
| `examples/temperature-measurement-app`         |                              | UNMAINTAINED |
| `examples/terms-and-conditions-app`            | James Swan                   |              |
| `examples/thermostat`                          |                              | UNMAINTAINED |
| `examples/thread-br-app`                       |                              | UNMAINTAINED |
| `examples/tv-app`                              | Chris DeCenzo, Lazar Kovacic |              |
| `examples/tv-casting-app`                      | Chris DeCenzo, Lazar Kovacic |              |
| `examples/virtual-device-app`                  |                              | UNMAINTAINED |
| `examples/water-leak-detector-app`             |                              | UNMAINTAINED |
| `examples/window-app`                          |                              | UNMAINTAINED |
