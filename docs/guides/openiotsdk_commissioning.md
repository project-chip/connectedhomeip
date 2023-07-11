# Commissioning Open IoT SDK devices

Matter devices based on Open IoT SDK reset into a ready for commissioning state.
This allows a controller to connect to them and set configuration options before
the device becomes available on the Matter network.

Open IoT SDK Matter devices, due to the connectivity setup, start already
connected to the IP network and do not require credentials provisioning.

## Building Matter controller

The [Matter controller](../../src/controller/README.md) is a client application
that allows commission and control of the Matter node.

The [POSIX CLI chip-tool](../../examples/chip-tool/README.md) is the recommended
Matter controller to use with Open IoT SDK devices.

To build `chip-tool` execute command:

```
scripts/examples/gn_build_example.sh examples/chip-tool out/chip-tool
```

## Device commissioning

Open IoT SDK examples require to setup a specific
[network environment](./openiotsdk_examples.md#networking-setup):

```
export TEST_NETWORK_NAME=OIStest
sudo ${MATTER_ROOT}/scripts/setup/openiotsdk/network_setup.sh -n $TEST_NETWORK_NAME restart
```

Both the device application and the controller should be started in a separate
terminal sessions and inside the created network namespace. Use the
`{MATTER_ROOT}/scripts/examples/scripts/run_in_ns.sh` helper script to execute
the specific shell command inside the network namespace.

Run the application in `device terminal` and use the network namespace:

```
${MATTER_ROOT}/scripts/examples/scripts/run_in_ns.sh ${TEST_NETWORK_NAME}ns
${MATTER_ROOT}/scripts/examples/openiotsdk_example.sh -C run -n ${TEST_NETWORK_NAME}tap
<example name>
```

After the device boots, it's in ready for commissioning mode and starts the mDNS
advertisement.

To commission the node in `client terminal` run Matter controller in the same
network namespace with pairing command:

```
${MATTER_ROOT}/scripts/run_in_ns.sh ${TEST_NETWORK_NAME}ns ./out/chip-tool/chip-tool pairing onnetwork-long <node_id> <pin_code> <discriminator>
```

The `node_id` is chosen by the user. `pin_code` and `discriminator` are node
parameters printed when running the application in the `device terminal`.

Example:

```
${MATTER_ROOT}/scripts/run_in_ns.sh ${TEST_NETWORK_NAME}ns ./out/chip-tool/chip-tool pairing onnetwork-long 123 20202021 3840
```

The application output a trace when commissioning succeeds. This trace is
displayed in the `device terminal`:

```
[INF] [SVR] Commissioning completed successfully
```

## Sending ZCL cluster commands

Before sending cluster commands [commission](#device-commissioning) the device.

To perform cluster action, run the Matter controller in the `client terminal`
and use the same network namespace:

```
${MATTER_ROOT}/scripts/run_in_ns.sh ${TEST_NETWORK_NAME}ns ./out/chip-tool/chip-tool <cluster_name> <command_name> [param1 param2 ...]
```

Example:

```
${MATTER_ROOT}/scripts/run_in_ns.sh ${TEST_NETWORK_NAME}ns ./out/chip-tool/chip-tool basicinformation read vendor-id 123 0
```
