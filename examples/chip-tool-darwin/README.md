# Matter chip-tool-darwin

An example application that uses Matter to send messages to a Matter server.

IMPORTANT: Must have an Apple developer signgin certificate. Information can be
found at [code-signing](https://developer.apple.com/support/code-signing/).

---

-   [Building the Example Application](#building-the-example-application)
-   [Using the Client to Request an Echo](#using-the-client-to-request-an-echo)

---

## Building the Example Application

See [the build guide](../../docs/guides/BUILDING.md#prerequisites) for general
background on build prerequisites.

Building the example application is quite straightforward.

```
scripts/examples/gn_build_example.sh examples/chip-tool-darwin SOME-PATH/
```

which puts the binary at `SOME-PATH/chip-tool-darwin`.

## Using the Client to commission a device

In order to send commands to a device, it must be commissioned with the client.
chip-tool-darwin currently only supports commissioning and remembering one
device at a time. The configuration state is stored in
`/tmp/chip_tool_config.ini`; deleting this and other `.ini` files in `/tmp` can
sometimes resolve issues due to stale configuration.

#### Commission a device

To initiate a client commissioning request to a device, run the built executable
and choose the pairing mode.

#### Pair a device over IP

The command below will pair devices with the provided IP, discriminator and
setup code.

    $ chip-tool-darwin pairing ethernet {NODE_ID_TO_ASSIGN} 20202021 3840 {IP_ADDRESS}

In this case, the device will be assigned node id `${NODE_ID_TO_ASSIGN}` (which
must be a decimal number or a 0x-prefixed hex number).

### Forget the currently-commissioned device

    $ chip-tool-darwin pairing unpair

## Using the Client to Send Matter Commands

To use the Client to send Matter commands, run the built executable and pass it
the target cluster name, the target command name as well as an endpoint id.

The endpoint id must be between 1 and 240.

    $ chip-tool-darwin onoff on 1

The client will send a single command packet and then exit.

### How to get the list of supported clusters

To get the list of supported clusters, run the built executable without any
arguments.

    $ chip-tool-darwin

Example output:

```bash
Usage:
  ./chip-tool-darwin cluster_name command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Clusters:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * barriercontrol                                                                    |
  | * basic                                                                             |
  | * colorcontrol                                                                      |
  | * doorlock                                                                          |
  | * groups                                                                            |
  | * iaszone                                                                           |
  | * identify                                                                          |
  | * levelcontrol                                                                      |
  | * onoff                                                                             |
  | * pairing                                                                           |
  | * payload                                                                           |
  | * scenes                                                                            |
  | * temperaturemeasurement                                                            |
  +-------------------------------------------------------------------------------------+
```

### How to get the list of supported commands for a specific cluster

To get the list of commands for a specific cluster, run the built executable
with the target cluster name.

    $ chip-tool-darwin onoff

### How to get the list of supported attributes for a specific cluster

To the the list of attributes for a specific cluster, run the built executable
with the target cluster name and the `read` command name.

    $ chip-tool-darwin onoff read

### How to get the list of parameters for a command

To get the list of parameters for a specific command, run the built executable
with the target cluster name and the target command name

    $ chip-tool-darwin onoff on

# Command Reference

## Command Details

<details><summary>All Clusters</summary><p>

<details><summary>Account Login</summary><p>

```bash
Usage:
  ./chip-tool-darwin account login command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * get-setup-p-i-n-request                                                           |
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
  ./chip-tool-darwin application launcher command_name [param1 param2 ...]

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
  ./chip-tool-darwin audio output command_name [param1 param2 ...]

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
  ./chip-tool-darwin barrier control command_name [param1 param2 ...]

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
  ./chip-tool-darwin bridged actions command_name [param1 param2 ...]

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
  ./chip-tool-darwin color control command_name [param1 param2 ...]

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
  ./chip-tool-darwin content launcher command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * launch-content-request                                                            |
  | * launch-u-r-l-request                                                              |
  +-------------------------------------------------------------------------------------+
```

</details>
<details><summary>Diagnostic Logs</summary><p>

```bash
Usage:
  ./chip-tool-darwin diagnostic logs command_name [param1 param2 ...]

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
  ./chip-tool-darwin door lock command_name [param1 param2 ...]

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
  ./chip-tool-darwin ethernet network diagnostics command_name [param1 param2 ...]

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
  ./chip-tool-darwin general commissioning command_name [param1 param2 ...]

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
  ./chip-tool-darwin group key management command_name [param1 param2 ...]

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
  ./chip-tool-darwin keypad input command_name [param1 param2 ...]

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
  ./chip-tool-darwin level control command_name [param1 param2 ...]

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
  ./chip-tool-darwin low power command_name [param1 param2 ...]

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
  ./chip-tool-darwin media input command_name [param1 param2 ...]

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
  ./chip-tool-darwin media playback command_name [param1 param2 ...]

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
  ./chip-tool-darwin mode select command_name [param1 param2 ...]

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
  ./chip-tool-darwin network commissioning command_name [param1 param2 ...]

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
  ./chip-tool-darwin ota software update provider command_name [param1 param2 ...]

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
  ./chip-tool-darwin ota software update requestor command_name [param1 param2 ...]

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
  ./chip-tool-darwin operational credentials command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * add-n-o-c                                                                         |
  | * add-trusted-root-certificate                                                      |
  | * attestation-request                                                               |
  | * c-s-r-request                                                                     |
  | * certificate-chain-request                                                         |
  | * remove-fabric                                                                     |
  | * remove-trusted-root-certificate                                                   |
  | * update-fabric-label                                                               |
  | * update-n-o-c                                                                      |
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
  ./chip-tool-darwin software diagnostics command_name [param1 param2 ...]

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
  ./chip-tool-darwin target navigator command_name [param1 param2 ...]

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
  ./chip-tool-darwin test cluster command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * simple-struct-echo-request                                                        |
  | * test                                                                              |
  | * test-add-arguments                                                                |
  | * test-emit-test-event-request                                                      |
  | * test-enums-request                                                                |
  | * test-list-int8-u-argument-request                                                 |
  | * test-list-int8-u-reverse-request                                                  |
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
  ./chip-tool-darwin thread network diagnostics command_name [param1 param2 ...]

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
  ./chip-tool-darwin wifi network diagnostics command_name [param1 param2 ...]

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
  ./chip-tool-darwin window covering command_name [param1 param2 ...]

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
</details>
