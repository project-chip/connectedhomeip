# Command Details

<details><summary>Account Login</summary><p>

```bash
Usage:
  ./chip-tool-darwin accountlogin command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * get-setup-pinrequest                                                              |
  | * login-request                                                                     |
  | * logout-request                                                                    |
  +-------------------------------------------------------------------------------------+
```

</details>
<details><summary>AdministratorCommissioning</summary><p>

```bash
Usage:
  ./chip-tool-darwin administratorcommissioning command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * open-basic-commissioning-window                                                   |
  | * open-commissioning-window                                                         |
  | * revoke-commissioning                                                              |
  +-------------------------------------------------------------------------------------+
```

</details>
<details><summary>Application Launcher</summary><p>

```bash
Usage:
  ./chip-tool-darwin applicationlauncher command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * hide-app-request                                                                  |
  | * launch-app-request                                                                |
  | * stop-app-request                                                                  |
  +-------------------------------------------------------------------------------------+
```

</details>
<details><summary>Audio Output</summary><p>

```bash
Usage:
  ./chip-tool-darwin audiooutput command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * rename-output-request                                                             |
  | * select-output-request                                                             |
  +-------------------------------------------------------------------------------------+
```

</details>
<details><summary>Barrier Control</summary><p>

```bash
Usage:
  ./chip-tool-darwin barriercontrol command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * barrier-control-go-to-percent                                                     |
  | * barrier-control-stop                                                              |
  +-------------------------------------------------------------------------------------+
```

</details>
<details><summary>Binding</summary><p>

```bash
Usage:
  ./chip-tool-darwin binding command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * bind                                                                              |
  | * unbind                                                                            |
  +-------------------------------------------------------------------------------------+
```

</details>
<details><summary>Bridged Actions</summary><p>

```bash
Usage:
  ./chip-tool-darwin bridgedactions command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * disable-action                                                                    |
  | * disable-action-with-duration                                                      |
  | * enable-action                                                                     |
  | * enable-action-with-duration                                                       |
  | * instant-action                                                                    |
  | * instant-action-with-transition                                                    |
  | * pause-action                                                                      |
  | * pause-action-with-duration                                                        |
  | * resume-action                                                                     |
  | * start-action                                                                      |
  | * start-action-with-duration                                                        |
  | * stop-action                                                                       |
  +-------------------------------------------------------------------------------------+
```

</details>
<details><summary>Channel</summary><p>

```bash
Usage:
  ./chip-tool-darwin channel command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * change-channel-by-number-request                                                  |
  | * change-channel-request                                                            |
  | * skip-channel-request                                                              |
  +-------------------------------------------------------------------------------------+
```

</details>
<details><summary>Color Control</summary><p>

```bash
Usage:
  ./chip-tool-darwin colorcontrol command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * color-loop-set                                                                    |
  | * enhanced-move-hue                                                                 |
  | * enhanced-move-to-hue                                                              |
  | * enhanced-move-to-hue-and-saturation                                               |
  | * enhanced-step-hue                                                                 |
  | * move-color                                                                        |
  | * move-color-temperature                                                            |
  | * move-hue                                                                          |
  | * move-saturation                                                                   |
  | * move-to-color                                                                     |
  | * move-to-color-temperature                                                         |
  | * move-to-hue                                                                       |
  | * move-to-hue-and-saturation                                                        |
  | * move-to-saturation                                                                |
  | * step-color                                                                        |
  | * step-color-temperature                                                            |
  | * step-hue                                                                          |
  | * step-saturation                                                                   |
  | * stop-move-step                                                                    |
  +-------------------------------------------------------------------------------------+
```

</details>
<details><summary>Content Launcher</summary><p>

```bash
Usage:
  ./chip-tool-darwin contentlauncher command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * launch-content-request                                                            |
  | * launch-urlrequest                                                                 |
  +-------------------------------------------------------------------------------------+
```

</details>
<details><summary>Diagnostic Logs</summary><p>

```bash
Usage:
  ./chip-tool-darwin diagnosticlogs command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * retrieve-logs-request                                                             |
  +-------------------------------------------------------------------------------------+
```

</details>
<details><summary>Door Lock</summary><p>

```bash
Usage:
  ./chip-tool-darwin doorlock command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * clear-credential                                                                  |
  | * clear-user                                                                        |
  | * clear-week-day-schedule                                                           |
  | * clear-year-day-schedule                                                           |
  | * get-credential-status                                                             |
  | * get-user                                                                          |
  | * get-week-day-schedule                                                             |
  | * get-year-day-schedule                                                             |
  | * lock-door                                                                         |
  | * set-credential                                                                    |
  | * set-user                                                                          |
  | * set-week-day-schedule                                                             |
  | * set-year-day-schedule                                                             |
  | * unlock-door                                                                       |
  | * unlock-with-timeout                                                               |
  +-------------------------------------------------------------------------------------+
```

</details>
<details><summary>Ethernet Network Diagnostics</summary><p>

```bash
Usage:
  ./chip-tool-darwin ethernetnetworkdiagnostics command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * reset-counts                                                                      |
  +-------------------------------------------------------------------------------------+
```

</details>
<details><summary>General Commissioning</summary><p>

```bash
Usage:
  ./chip-tool-darwin generalcommissioning command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * arm-fail-safe                                                                     |
  | * commissioning-complete                                                            |
  | * set-regulatory-config                                                             |
  +-------------------------------------------------------------------------------------+
```

</details>
<details><summary>Group Key Management</summary><p>

```bash
Usage:
  ./chip-tool-darwin groupkeymanagement command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * key-set-read                                                                      |
  | * key-set-read-all-indices                                                          |
  | * key-set-remove                                                                    |
  | * key-set-write                                                                     |
  +-------------------------------------------------------------------------------------+
```

</details>
<details><summary>Groups</summary><p>

```bash
Usage:
  ./chip-tool-darwin groups command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * add-group                                                                         |
  | * add-group-if-identifying                                                          |
  | * get-group-membership                                                              |
  | * remove-all-groups                                                                 |
  | * remove-group                                                                      |
  | * view-group                                                                        |
  +-------------------------------------------------------------------------------------+
```

</details>
<details><summary>Identify</summary><p>

```bash
Usage:
  ./chip-tool-darwin identify command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * identify                                                                          |
  | * identify-query                                                                    |
  | * trigger-effect                                                                    |
  +-------------------------------------------------------------------------------------+
```

</details>
<details><summary>Keypad Input</summary><p>

```bash
Usage:
  ./chip-tool-darwin keypadinput command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * send-key-request                                                                  |
  +-------------------------------------------------------------------------------------+
```

</details>
<details><summary>Level Control</summary><p>

```bash
Usage:
  ./chip-tool-darwin levelcontrol command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * move                                                                              |
  | * move-to-level                                                                     |
  | * move-to-level-with-on-off                                                         |
  | * move-with-on-off                                                                  |
  | * step                                                                              |
  | * step-with-on-off                                                                  |
  | * stop                                                                              |
  | * stop-with-on-off                                                                  |
  +-------------------------------------------------------------------------------------+
```

</details>
<details><summary>Low Power</summary><p>

```bash
Usage:
  ./chip-tool-darwin lowpower command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * sleep                                                                             |
  +-------------------------------------------------------------------------------------+
```

</details>
<details><summary>Media Input</summary><p>

```bash
Usage:
  ./chip-tool-darwin mediainput command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * hide-input-status-request                                                         |
  | * rename-input-request                                                              |
  | * select-input-request                                                              |
  | * show-input-status-request                                                         |
  +-------------------------------------------------------------------------------------+
```

</details>
<details><summary>Media Playback</summary><p>

```bash
Usage:
  ./chip-tool-darwin mediaplayback command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * fast-forward-request                                                              |
  | * next-request                                                                      |
  | * pause-request                                                                     |
  | * play-request                                                                      |
  | * previous-request                                                                  |
  | * rewind-request                                                                    |
  | * seek-request                                                                      |
  | * skip-backward-request                                                             |
  | * skip-forward-request                                                              |
  | * start-over-request                                                                |
  | * stop-request                                                                      |
  +-------------------------------------------------------------------------------------+
```

</details>
<details><summary>Mode Select</summary><p>

```bash
Usage:
  ./chip-tool-darwin modeselect command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * change-to-mode                                                                    |
  +-------------------------------------------------------------------------------------+
```

</details>
<details><summary>Network Commissioning</summary><p>

```bash
Usage:
  ./chip-tool-darwin networkcommissioning command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * add-or-update-thread-network                                                      |
  | * add-or-update-wi-fi-network                                                       |
  | * connect-network                                                                   |
  | * remove-network                                                                    |
  | * reorder-network                                                                   |
  | * scan-networks                                                                     |
  +-------------------------------------------------------------------------------------+
```

</details>
<details><summary>OTA Software Update Provider</summary><p>

```bash
Usage:
  ./chip-tool-darwin otasoftwareupdateprovider command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * apply-update-request                                                              |
  | * notify-update-applied                                                             |
  | * query-image                                                                       |
  +-------------------------------------------------------------------------------------+
```

</details>
<details><summary>OTA Software Update Requestor</summary><p>

```bash
Usage:
  ./chip-tool-darwin otasoftwareupdaterequestor command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * announce-ota-provider                                                             |
  +-------------------------------------------------------------------------------------+
```

</details>
<details><summary>On/Off</summary><p>

```bash
Usage:
  ./chip-tool-darwin on/off command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * off                                                                               |
  | * off-with-effect                                                                   |
  | * on                                                                                |
  | * on-with-recall-global-scene                                                       |
  | * on-with-timed-off                                                                 |
  | * toggle                                                                            |
  +-------------------------------------------------------------------------------------+
```

</details>
<details><summary>Operational Credentials</summary><p>

```bash
Usage:
  ./chip-tool-darwin operationalcredentials command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * add-noc                                                                           |
  | * add-trusted-root-certificate                                                      |
  | * attestation-request                                                               |
  | * csrrequest                                                                        |
  | * certificate-chain-request                                                         |
  | * remove-fabric                                                                     |
  | * remove-trusted-root-certificate                                                   |
  | * update-fabric-label                                                               |
  | * update-noc                                                                        |
  +-------------------------------------------------------------------------------------+
```

</details>
<details><summary>Scenes</summary><p>

```bash
Usage:
  ./chip-tool-darwin scenes command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * add-scene                                                                         |
  | * get-scene-membership                                                              |
  | * recall-scene                                                                      |
  | * remove-all-scenes                                                                 |
  | * remove-scene                                                                      |
  | * store-scene                                                                       |
  | * view-scene                                                                        |
  +-------------------------------------------------------------------------------------+
```

</details>
<details><summary>Software Diagnostics</summary><p>

```bash
Usage:
  ./chip-tool-darwin softwarediagnostics command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * reset-watermarks                                                                  |
  +-------------------------------------------------------------------------------------+
```

</details>
<details><summary>Target Navigator</summary><p>

```bash
Usage:
  ./chip-tool-darwin targetnavigator command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * navigate-target-request                                                           |
  +-------------------------------------------------------------------------------------+
```

</details>
<details><summary>Test Cluster</summary><p>

```bash
Usage:
  ./chip-tool-darwin testcluster command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * simple-struct-echo-request                                                        |
  | * test                                                                              |
  | * test-add-arguments                                                                |
  | * test-emit-test-event-request                                                      |
  | * test-enums-request                                                                |
  | * test-list-int8uargument-request                                                   |
  | * test-list-int8ureverse-request                                                    |
  | * test-list-nested-struct-list-argument-request                                     |
  | * test-list-struct-argument-request                                                 |
  | * test-nested-struct-argument-request                                               |
  | * test-nested-struct-list-argument-request                                          |
  | * test-not-handled                                                                  |
  | * test-nullable-optional-request                                                    |
  | * test-simple-optional-argument-request                                             |
  | * test-specific                                                                     |
  | * test-struct-argument-request                                                      |
  | * test-unknown-command                                                              |
  | * timed-invoke-request                                                              |
  +-------------------------------------------------------------------------------------+
```

</details>
<details><summary>Thermostat</summary><p>

```bash
Usage:
  ./chip-tool-darwin thermostat command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * clear-weekly-schedule                                                             |
  | * get-relay-status-log                                                              |
  | * get-weekly-schedule                                                               |
  | * set-weekly-schedule                                                               |
  | * setpoint-raise-lower                                                              |
  +-------------------------------------------------------------------------------------+
```

</details>
<details><summary>Thread Network Diagnostics</summary><p>

```bash
Usage:
  ./chip-tool-darwin threadnetworkdiagnostics command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * reset-counts                                                                      |
  +-------------------------------------------------------------------------------------+
```

</details>
<details><summary>WiFi Network Diagnostics</summary><p>

```bash
Usage:
  ./chip-tool-darwin wifinetworkdiagnostics command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * reset-counts                                                                      |
  +-------------------------------------------------------------------------------------+
```

</details>
<details><summary>Window Covering</summary><p>

```bash
Usage:
  ./chip-tool-darwin windowcovering command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * down-or-close                                                                     |
  | * go-to-lift-percentage                                                             |
  | * go-to-lift-value                                                                  |
  | * go-to-tilt-percentage                                                             |
  | * go-to-tilt-value                                                                  |
  | * stop-motion                                                                       |
  | * up-or-open                                                                        |
  +-------------------------------------------------------------------------------------+
```

</details>
